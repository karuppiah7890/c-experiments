#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

int main()
{
    // server file descriptor
    int server_fd;
    int client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    char buffer[1024] = {0};

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

    // 4. Accept a client connection
    client_socket = accept(server_fd,
                           (struct sockaddr *)&address,
                           (socklen_t *)&addrlen);

    if (client_socket < 0)
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected!\n");

    printf("Client Socket File Descriptor: %d\n", client_socket);

    // 5. Read data from client
    read(client_socket, buffer, sizeof(buffer));
    printf("Client says: %s\n", buffer);

    // 6. Send response
    char *response = "Hello from server";
    send(client_socket, response, strlen(response), 0);

    printf("Response sent\n");

    close(client_socket);
    close(server_fd);

    return 0;
}