#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

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

    struct sockaddr_in client_address;
    int client_addrlen;
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

    // 5. Read data from client
    read(client_socket, buffer, sizeof(buffer));
    printf("Client says: %s\n", buffer);

    // 6. Send response
    char *response = "Hello from server";
    send(client_socket, response, strlen(response), 0);

    printf("Response sent\n");

    if (close(client_socket) < 0)
    {
        perror("error closing client socket file descriptor");
    }
    if (close(server_fd) < 0)
    {
        perror("error closing client socket file descriptor");
    }

    return 0;
}