#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <thread>

#include "../model/dto/SocketData.cpp"

#define PORT 1026

int main(int argc, char const* argv[])
{

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    std::vector<std::thread> threads;
    std::vector<DroneData> drones;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    bool run = true;
    while (run) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("new connection failed");
            exit(EXIT_FAILURE);
        }

        threads.emplace_back([&new_socket, &drones] {
            int valread;
            char buffer[1024] = { 0 };
            std::string hello = "Hello from server";


            try {
                send(new_socket, hello.c_str(), hello.length(), 0);
                valread = read(new_socket, buffer, 1024);
                printf("%s\n", buffer);

                DroneData droneData = {0};
                long client_data = recv(new_socket, &droneData, sizeof(droneData), 0);
                std::cout << "droneId: " << droneData.droneId << std::endl << "port: " << droneData.port << std::endl << "action: " << droneData.action << std::endl;
                drones.push_back(droneData);

                std::string client_ans = "The data is received from the server";
                send(new_socket, client_ans.c_str(), client_ans.length(), 0);

                close(new_socket);
            } catch (...) {
                close(new_socket);
            }
        });

        char x;
        std::cout << "Do you want the server to terminate (Y/N)" << std::endl;
        std::cin >> x;
        if (x == 'Y') {
            run = false;
            shutdown(server_fd, SHUT_RDWR);
        }
    }

    for (auto& thread : threads) {
        thread.join();
    }
    return 0;
}

