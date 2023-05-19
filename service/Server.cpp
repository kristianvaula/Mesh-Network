//
// Created by HansMagne Asheim on 16/05/2023.
//
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <mutex>

#include "/nettverksprog/mesh-network/model/enums/ActionType.h"
#include "/nettverksprog/mesh-network/model/NodeList.h"
#include "/nettverksprog/mesh-network/service/IpUtils.h"

#define PORT 1033

class Server {
private:
    std::unordered_map<std::string, ActionType> actionTypes;
    std::vector<std::thread> threads;
    NodeList nodeList;
    int server_fd;
    bool run;
    std::mutex nodeList_mutex;
    IpUtils ipUtils;

public:
    Server()
        : actionTypes(),
          threads(),
          nodeList(3),//sets the mesh network size to 5
          server_fd(-1),
          nodeList_mutex(),
          ipUtils(),
          run(true) {};

    void start() {
        createSocket();
        bindSocket();
        listenForConnections();
        handleConnections();
        cleanup();
    }

    void stop() {
        run = false;
        shutdown(server_fd, SHUT_RDWR);
    }

private:
    void createSocket() {
        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
    }

    void bindSocket() {
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        // Forcefully attaching socket to the port
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
    }

    void listenForConnections() {
        if (listen(server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
    }

    void handleConnection(int new_socket) {
        int valread;
        char buffer[1024] = { 0 };
        std::string hello = "Hello from server";

        try {
            send(new_socket, hello.c_str(), hello.length(), 0);
            valread = read(new_socket, buffer, sizeof(buffer));
            printf("%s\n", buffer);

            NodeData nodeData = {0};
            long client_data = recv(new_socket, &nodeData, sizeof(nodeData), 0);
            std::cout << "droneId: " << nodeData.nodeId << std::endl
            << "port: " << nodeData.port << std::endl
            << "ipAddress: " << nodeData.ipAddress << std::endl
            << "action: " << nodeData.action << std::endl;
            if (actionTypes[nodeData.action] == ActionType::HELLO) {
                Node node(nodeData);
                {//aquire lock
                    std::unique_lock<std::mutex> lock(this->nodeList_mutex);
                    if (!nodeList.isMeshFull()) {
                        Node node = nodeList.addNodeToMesh(nodeData);
                        NodeData nodeData = {0};
                        nodeData.nodeId = node.getNodeId();
                        Node* connectedInnerNode = nodeList.getConnectedInnerNode(node);
                        if (connectedInnerNode != nullptr) {
                            nodeData.port = connectedInnerNode->getPort();
                            strcpy(nodeData.ipAddress, connectedInnerNode->getIpAddress().c_str());
                        } else { //the node is master node
                            nodeData.port = PORT;
                            char ipAddress[256];
                            ipUtils.getIPAddress(ipAddress, sizeof(ipAddress));
                            strcpy(nodeData.ipAddress, ipAddress);
                        }
                        char action[256];
                        strcpy(action, "MOVETO_");
                        strcat(action, std::to_string(node.getXPosition()).c_str());

                        if(node.getPriority() == Priority::HIGH) {                            
                            nodeList.setSocketToMasterNode(new_socket);
                        }

                        strcpy(nodeData.action, action);
                        send(new_socket, &nodeData, sizeof(nodeData), 0);
                    } else {
                        nodeList.addNode(node);
                    }
                }//release lock
            }                
            close(new_socket);
        } catch (...) {
            close(new_socket);
        }
    }

    void handleConnections() {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        actionTypes = {
            { "REMOVE_NODE", ActionType::REMOVE_NODE },
            { "MOVETO", ActionType::MOVETO },
            { "HELLO", ActionType::HELLO }
        };


        while (run) {
            int new_socket;
            if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
                perror("new connection failed");
                exit(EXIT_FAILURE);
            }

            threads.emplace_back([&] {
                handleConnection(new_socket);
            });

        }
    }

    void cleanup() {
        for (auto& thread : threads) {
            thread.join();
        }
    }
};

int main(int argc, char const* argv[])
{
    Server server;
    server.start();

    return 0;
}
