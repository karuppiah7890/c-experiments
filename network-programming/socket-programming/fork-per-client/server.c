#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

void handle_client(int client_socket)
{
    char buffer[1024] = {0};

    printf("Client connected!\n");

    printf("Client Socket File Descriptor: %d\n", client_socket);

    // 5. Read data from client
    read(client_socket, buffer, sizeof(buffer));
    printf("Client says: %s\n", buffer);

    // 6. Send response
    char *response = "Hello from server";
    send(client_socket, response, strlen(response), 0);

    printf("Response sent\n");
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
        // 4. Accept a client connection
        client_socket = accept(server_fd,
                               (struct sockaddr *)&address,
                               (socklen_t *)&addrlen);

        if (client_socket < 0)
        {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

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

            // If we wait for the child process to complete in the main thread, then the server will be stuck waiting for the child process to complete before accepting another connection. So, it becomes a blocking call to use `waitpid()`
            // int status;
            // printf("Parent process (PID: %d) created child (PID: %d) and is waiting for it to finish.\n", getpid(), pid);

            // // 3. Wait for the child process to terminate
            // if (waitpid(pid, &status, 0) == -1)
            // {
            //     perror("waitpid failed");
            // }

            // if (WIFEXITED(status))
            // {
            //     printf("Child process exited with status %d.\n", WEXITSTATUS(status));
            // }
            // else
            // {
            //     printf("Child process terminated abnormally.\n");
            // }
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
