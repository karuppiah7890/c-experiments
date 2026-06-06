#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

void print_sockaddr_in(const struct sockaddr_in *addr)
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

void handle_client(int client_socket)
{
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
    send(client_socket, response, strlen(response), 0);

    printf("Response sent\n");
}

void *wait_for_child_process(void *arg)
{
    pid_t pid = *(pid_t *)arg;
    printf("Waiting on child process ID: %d\n", pid);
    int status;

    // Wait for the child process to terminate
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid failed");
    }

    if (WIFEXITED(status))
    {
        printf("Child process %d exited with status %d.\n", pid, WEXITSTATUS(status));
    }
    else
    {
        printf("Child process %d terminated abnormally.\n", pid);
    }

    return NULL;
}

int main()
{
    // server file descriptor
    int server_fd;
    int client_socket;
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
    if (listen(server_fd, 5) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        struct sockaddr_in client_address;
        int client_addrlen = sizeof(client_address);
        // 4. Accept a client connection
        client_socket = accept(server_fd,
                               (struct sockaddr *)&client_address,
                               (socklen_t *)&client_addrlen);

        if (client_socket < 0)
        {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Client connected!\n");

        printf("Client Socket File Descriptor: %d\n", client_socket);

        print_sockaddr_in(&client_address);

        // fork() to handle client connections separately using separate
        // processes - child processes
        pid_t pid = fork();

        if (pid == -1)
        {
            // Error handling if fork() fails
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0)
        {
            // Child process

            // As child process does not need the server socket file descriptor
            // as that's only for accepting clients and server has a copy of
            // the file descriptor table which has the same file descriptor
            // value pointing to the actual kernel socket object reference
            // in the kernel and can accept clients with it. The kernel socket
            // object reference count is tracked - so, even if one kernel
            // socket object reference is there, the socket is not closed.
            // close(socket) simply removes the reference in the process
            // file descriptor table - so, if there are other references,
            // for example in the parent or child process since the
            // file descriptor tables are copied to child from parent
            // during fork(), then the kernel destroys the socket (garbage
            // collection) only if all references are removed
            if (close(server_fd) < 0)
            {
                perror("error closing server socket file descriptor in child process");
            }

            handle_client(client_socket);

            if (close(client_socket) < 0)
            {
                perror("error closing client socket file descriptor in child process");
            }

            exit(0);
        }
        else
        {
            // This block is executed by the parent process
            printf("pid of child process is: %d\n", pid);

            // Not waiting for the child process causes the child process to finish (exit) and become zombie processes in the process list - check using `ps aux` command and you will see processes with status `Z` and the process name with the keyword `defunct`

            // If we wait for the child process to complete in the main thread, then the server will be stuck waiting for the child process to complete before accepting another client connection. So, it becomes a blocking call to use `waitpid()`

            // So, we move the waiting to a thread! Not ideal or great, but it's the least we can do - better than not waiting at all causing zombie processes to build up in the process list cluttering the process list

            pthread_t tid;

            int status = pthread_create(&tid, NULL, wait_for_child_process, &pid);

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
        }

        // As the server process does not require the client socket
        // file descriptor as the client has access to it and child
        // process has reference to the kernel socket object so the
        // socket is not destroyed until all references are removed
        if (close(client_socket) < 0)
        {
            perror("error closing client socket file descriptor in parent process");
        }
    }

    if (close(server_fd) < 0)
    {
        perror("error closing server socket file descriptor in parent process");
    }

    return 0;
}
