#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080

int main()
{
    int sock = 0;
    struct sockaddr_in server_addr;

    char buffer[1024] = {0};

    // 1. Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    printf("Client Socket File Descriptor: %d\n", sock);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IP address text -> binary
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // 2. Connect to server
    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    // 3. Send message
    char *message = "Hello from client";
    send(sock, message, strlen(message), 0);

    // 4. Receive response
    recv(sock, buffer, sizeof(buffer), 0);

    printf("Server replied: %s\n", buffer);

    close(sock);

    return 0;
}