#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

#define DEBUG 0

struct node
{
    int value;
    struct node *next;
};

struct queue
{
    struct node *head;
    struct node *tail;
    int size;
};

bool is_queue_empty(struct queue *q)
{
    return q->size == 0;
}

void enqueue(struct queue *q, int value)
{
    struct node *node = malloc(sizeof(struct node));

    node->value = value;
    node->next = NULL;

    if (q->tail == NULL)
    {
        // here head MUST also be NULL actually
        q->head = node;
        q->tail = q->head;
        q->size = 1;
        return;
    }

    q->tail->next = node;
    q->tail = q->tail->next; // same as: q->tail = node;
    q->size++;
}

int dequeue(struct queue *q)
{
    if (q->head == NULL)
    {
        // here tail MUST also be NULL actually
        printf("head is null. nothing to dequeue!");
        return -1;
    }

    if (q->head == q->tail)
    {
        int value = q->head->value;
        free(q->head);
        q->head = NULL;
        q->tail = NULL;
        q->size = 0;
        return value;
    }

    struct node *top = q->head;
    q->head = q->head->next;
    int value = top->value;
    top->next = NULL;
    free(top);
    q->size--;
    return value;
}

void print_sockaddr_in(const struct sockaddr_in *addr)
{
    if (DEBUG)
    {
        char ip_str[INET_ADDRSTRLEN]; // Buffer to hold the IPv4 string

        // 1. Convert the binary IP address to a human-readable string
        if (inet_ntop(AF_INET, &(addr->sin_addr), ip_str, INET_ADDRSTRLEN) == NULL)
        {
            perror("inet_ntop failed");
            return;
        }

        // 2. Convert the port from network byte order to host byte order
        uint16_t port = ntohs(addr->sin_port);

        // 3. Print the results
        printf("IPv4 Address: %s\n", ip_str);
        printf("Port        : %d\n", port);
        printf("Family      : %d (AF_INET)\n", addr->sin_family);
    }
}

void handle_client(int client_socket)
{

    char buffer[1024] = {0};

    if (DEBUG)
    {
        printf("Client connected!\n");

        printf("Client Socket File Descriptor: %d\n", client_socket);
    }

    // 5. Read data from client
    // We read only 1024 bytes - which is good if the client sends less than
    // that, but if the client sends more than that, we need to read more
    // times using `read()` or `recv()` using a loop till the
    // received bytes is 0. The return value of `read()` and `recv()` is
    // the number of bytes received
    read(client_socket, buffer, sizeof(buffer));
    if (DEBUG)
    {
        printf("Client says: %s\n", buffer);
    }

    // 6. Send response
    char *response = "Hello from server";
    // For HTTP server:
    // char *response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    send(client_socket, response, strlen(response), 0);

    if (DEBUG)
    {
        printf("Response sent\n");
    }
    if (close(client_socket) < 0)
    {
        perror("error closing client socket file descriptor in child process");
    }
}

struct queue *socket_queue;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *worker(void *arg)
{
    for (;;)
    {
        pthread_mutex_lock(&mutex);

        while (is_queue_empty(socket_queue))
        {
            pthread_cond_wait(&cond, &mutex);
        }

        int fd = dequeue(socket_queue);

        pthread_mutex_unlock(&mutex);

        handle_client(fd);
    }
}

int main()
{
    // server file descriptor
    int server_fd;
    struct sockaddr_in address;

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    printf("Server Socket File Descriptor: %d\n", server_fd);

    int opt = 1;

    // TCP has a concept called TIME_WAIT
    // which causes "bind failed: Address already in use"
    // even though there's no other process bound to the
    // address. In order to overcome the error, we say
    // "allow local address reuse"
    setsockopt(server_fd,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 2. Bind socket to IP + PORT
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. Listen for incoming connections
    if (listen(server_fd, SOMAXCONN) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    socket_queue = malloc(sizeof(struct queue));

    // N is the number of CPU Cores
    // We are creating N worker threads, but we can also
    // create 2N, or 4N too, if the server is more I/O Bound
    // than CPU Bound where it does processing like JSON Parsing
    // calculating something, AI model, compress data etc
    int N = sysconf(_SC_NPROCESSORS_ONLN);

    int i = 1;

    while (i <= N)
    {
        pthread_t tid;

        int status = pthread_create(&tid, NULL, worker, NULL);

        if (status != 0)
        {
            fprintf(stderr,
                    "pthread_create failed: %s\n",
                    strerror(status));
        }
        else
        {
            pthread_detach(tid);
        }

        i++;
    }

    while (1)
    {
        struct sockaddr_in client_address;
        int client_addrlen = sizeof(client_address);
        // 4. Accept a client connection
        int client_socket = accept(server_fd,
                                   (struct sockaddr *)&client_address,
                                   (socklen_t *)&client_addrlen);

        if (client_socket < 0)
        {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        if (DEBUG)
        {
            printf("Client connected!\n");

            printf("Client Socket File Descriptor: %d\n", client_socket);

            print_sockaddr_in(&client_address);
        }

        pthread_mutex_lock(&mutex);

        enqueue(socket_queue, client_socket);

        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mutex);
    }

    if (close(server_fd) < 0)
    {
        perror("error closing server socket file descriptor in parent process");
    }

    return 0;
}
