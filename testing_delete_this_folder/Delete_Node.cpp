// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

#include "../model/dto/SocketData.cpp"

#define PORT 1025

int main(int argc, char const* argv[])
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    bool exit = false;
    while (!exit) {
        valread = read(client_fd, buffer, 1024);
        printf("%s\n", buffer);

        std::string hello = "Hello from node";
        send(client_fd, hello.c_str(), hello.length(), 0);

        NodeData nodeData = {0};
        nodeData.nodeId = 1;
        nodeData.port = PORT;
        nodeData.action = "ADD_NODE";

        send(client_fd, &nodeData, sizeof(nodeData), 0);

        char buffer[1024];
        int valread = recv(client_fd, buffer, sizeof(buffer), 0);
        if (valread > 0) {
            std::string serverResponse(buffer, buffer + valread);
            std::cout << "Server response: " << serverResponse << std::endl;
        }

        char x;
        std::cout << "Do you want the server to terminate (Y/N)" << std::endl;
        std::cin >> x;
        if (x == 'Y') {
            exit = true;
            // closing the connected socket
            close(client_fd);
        }
    }
    return 0;
}

