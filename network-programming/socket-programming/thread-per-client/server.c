#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define PORT 8080

void *handle_client(void *arg)
{
    printf(
        "worker. addr=%p fd=%d\n",
        arg,
        *(int *)arg);

    int client_socket = *(int *)arg;
    free(arg);

    char buffer[1024] = {0};

    printf("Client connected!\n");

    printf("Client Socket File Descriptor: %d\n", client_socket);

    // 5. Read data from client
    // We read only 1024 bytes - which is good if the client sends less than
    // that, but if the client sends more than that, we need to read more
    // times using `read()` or `recv()` using a loop till the
    // received bytes is 0. The return value of `read()` and `recv()` is
    // the number of bytes received
    read(client_socket, buffer, sizeof(buffer));
    printf("Client says: %s\n", buffer);

    // 6. Send response
    char *response = "Hello from server";
    // For HTTP server:
    // char *response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    send(client_socket, response, strlen(response), 0);

    printf("Response sent\n");
    if (close(client_socket) < 0)
    {
        perror("error closing client socket file descriptor in child process");
    }

    return NULL;
}

int main()
{
    // server file descriptor
    int server_fd;
    int client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

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
    if (listen(server_fd, 5) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        int *client_socket = malloc(sizeof(int));
        // 4. Accept a client connection
        *client_socket = accept(server_fd,
                                (struct sockaddr *)&address,
                                (socklen_t *)&addrlen);

        if (*client_socket < 0)
        {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        printf(
            "accept. fd=%d addr=%p\n",
            *client_socket,
            (void *)client_socket);

        pthread_t tid;

        int status = pthread_create(&tid, NULL, handle_client, client_socket);

        if (status != 0)
        {
            fprintf(stderr,
                    "pthread_create failed: %s\n",
                    strerror(status));
        } else {
            pthread_detach(tid);
        }
    }

    if (close(server_fd) < 0)
    {
        perror("error closing server socket file descriptor in parent process");
    }

    return 0;
}
