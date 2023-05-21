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

#define PORT 1081

/*
    TODO:
    - Sjekke close_socket
    - sjekke joining
    - sjekke inifinte prining når en client avsluttes
    - implement method to close connection with a node. The close will happen on request from the node
*/
class Server {
private:
    std::unordered_map<std::string, ActionType> actionTypes;
    std::vector<std::thread> threads;
    NodeList nodeList;
    int server_fd;
    bool run;
    std::mutex nodeList_mutex;
    IpUtils ipUtils;
    std::thread inputThread;

public:
    Server()
        : actionTypes(),
          threads(),
          nodeList(5),//sets the mesh network size to 5
          server_fd(-1),
          nodeList_mutex(),
          ipUtils(),
          run(true) {};

    void start() {
        createSocket();
        bindSocket();
        listenForConnections();
        inputThread = std::thread([&] {
            handleServerInput();
        });
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

    void displayMenu() {
        std::cout << "Press 'q' if you want the server to terminate" << std::endl
                << "Press 'r' if you want to remove a node from the mesh network" << std::endl;
    }

    void handleServerInput() {
        bool serverInput = true;
        char x;

        while (serverInput) {
            displayMenu();
            std::cin >> x;
            if (x == 'q') {
                serverInput = false;
                stop();
            } else if (x == 'r') {
                int nodeId;
                std::cout << "Type in the nodeId to the node you want to remove" << std::endl;
                std::cin >> nodeId;
                std::cout << "Recived request to remove nodeId: " << nodeId << std::endl;
                removeNode(nodeId);
            }
        }
    }

    NodeData formatNodeToSend(Node* node, int new_socket) {
        NodeData nodeData = {0};
        nodeData.nodeId = node->getNodeId();
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
        strcat(action, std::to_string(node->getXPosition()).c_str());
        if(node->getPriority() == Priority::HIGH) {                            
            nodeList.setSocketToMasterNode(new_socket);
        }
        strcpy(nodeData.action, action);
        return nodeData;
    }

    void handleConnection(int new_socket) {
        int valread;
        char buffer[1024] = { 0 };
        bool clientConnection = true;
        char x;
        std::string hello = "Hello from server";
        send(new_socket, hello.c_str(), hello.length(), 0);
        valread = read(new_socket, buffer, sizeof(buffer));
        printf("%s\n", buffer);

        while (run) {
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
                        nodeList.addNodeToMesh(node);
                    } else {
                        nodeList.addNode(node);
                    }
                }//release lock
                Node* nodeListItem = nodeList.getNode(nodeData.nodeId);
                if (nodeListItem->getPriority() != Priority::NONE) {
                    NodeData formatedNodeData = formatNodeToSend(nodeListItem, new_socket);
                    send(new_socket, &formatedNodeData, sizeof(formatedNodeData), 0);
                }
            } else if (actionTypes[std::string(nodeData.action).substr(0, 7)] == ActionType::REPLACE) {
                std::string action = std::string(nodeData.action);
                int replacementNodeId = std::stoi(action.substr(8));
                {//aquire lock
                    std::unique_lock<std::mutex> lock(this->nodeList_mutex);
                    nodeList.replaceNode(nodeData.nodeId, replacementNodeId);
                }//release lock
            }
        }          
    }

    void removeNode(int nodeId) {
        int masterNode_socket = nodeList.getSocketToMasterNode();
        std::cout << "masterNode_socket: " << masterNode_socket << std::endl;
        if (masterNode_socket != -1 && nodeList.isNodeInMesh(nodeId)) {
            NodeData nodeData = {0};
            nodeData.nodeId = nodeId;
            nodeData.port = PORT;
            char ipAddress[256];
            ipUtils.getIPAddress(ipAddress, sizeof(ipAddress));
            strcpy(nodeData.ipAddress, ipAddress);
            strcpy(nodeData.action, "REMOVE_NODE");
            std::cout << "REMOVE NODE" << std::endl;
            std::cout << "droneId: " << nodeData.nodeId << std::endl
                << "port: " << nodeData.port << std::endl
                << "ipAddress: " << nodeData.ipAddress << std::endl
                << "action: " << nodeData.action << std::endl;
            send(masterNode_socket, &nodeData, sizeof(nodeData), 0);
        }
    }

    void handleConnections() {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        actionTypes = {
            { "REMOVE_NODE", ActionType::REMOVE_NODE },
            { "MOVETO", ActionType::MOVETO },
            { "HELLO", ActionType::HELLO },
            { "REPLACE", ActionType::REPLACE }
        };

        while (run) {
            int new_socket;
            char x;
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
        inputThread.join();
    }
};

int main(int argc, char const* argv[]) {
    Server server;
    server.start();

    return 0;
}
