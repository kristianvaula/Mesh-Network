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
#include <atomic>

#include "../model/enums/ActionType.hpp"
#include "../model/NodeList.hpp"
#include "IpUtils.hpp"

#define PORT 1088

/*
    TODO:
    - Sjekke close_socket: the server socket is never terminated
    - sjekke joining
*/
class Server {
private:
    std::unordered_map<std::string, ActionType> actionTypes;
    std::vector<std::thread> threads;
    NodeList nodeList;
    int server_fd;
    std::atomic<bool> run;
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
          run(true) {}

    void start() {
        createSocket();
        bindSocket();
        listenForConnections();
        inputThread = std::thread([&] {
            handleServerInput();
        });
        handleConnections();
    }

    void stop() {
        run.store(false);
        cleanup(); //Is this correct placement?
        shutdown(server_fd, SHUT_RDWR);
    }

private:
    void createSocket() {
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
        std::cout << "[Server] Server is running on port: " << PORT << std::endl
            << "[Server] Press 'q' if you want the server to terminate" << std::endl
            << "[Server] Press 'r' if you want to remove a node from the mesh network" << std::endl;
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
                std::cout << "[Server] Type in the nodeId to the node you want to remove" << std::endl;
                std::cin >> nodeId;
                std::cout << "[Server] Recived request to remove nodeId: " << nodeId << std::endl;
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
        strcpy(nodeData.action, action);
        if(node->getPriority() == Priority::HIGH) {                            
            nodeList.setSocketToMasterNode(new_socket);
        }
        return nodeData;
    }

    void handleConnection(int new_socket) {
        int valread;
        bool clientConnection = true;

        while (clientConnection && run.load()) {
            NodeData nodeData = {0};
            valread = recv(new_socket, &nodeData, sizeof(nodeData), 0);

            if (valread <= 0) {
                clientConnection = false;
                std::cout << "[Server: clientThread] Closing client conection" << std::endl;
                close(new_socket);
                return;
            } 
            
            std::cout << "[Server: clientThread] Recieve a message from client" << std::endl;
            std::cout << "[Server: clientThread] droneId: " << nodeData.nodeId << ", port: " << nodeData.port << ", ipAddress: " << nodeData.ipAddress
            << ", action: " << nodeData.action << std::endl;
            
            ActionType action = actionFromString(nodeData.action);
            switch(action) {
                case ActionType::HELLO: {
                    if (nodeList.isNodeInList(nodeData.nodeId)) {
                        std::cerr << "[Server: clientThread] The nodeId: " << nodeData.nodeId << " is already registered" << std::endl;
                        break;
                    }

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
                    break;
                }
                case ActionType::REPLACE: {
                    std::string action = std::string(nodeData.action);
                    size_t underscorePos = action.find('_'); 
                    if (underscorePos == std::string::npos) {
                        break;
                    }
                    std::string arg = action.substr(underscorePos+1); 
                    std::cout << "Arg: " << arg << std::endl; 
                    try{
                        int replacementNodeId = std::stoi(arg); 
                        {//aquire lock
                            std::unique_lock<std::mutex> lock(this->nodeList_mutex);
                            nodeList.replaceNode(nodeData.nodeId, replacementNodeId);
                        }//release lock
                    }
                    catch(std::invalid_argument e){
                        std::cerr << "[Server: clientThread] Received invalid positon" << std::endl; 
                    }
                    catch(std::out_of_range e){
                        std::cerr << "[Server: clientThread] Received invalid positon" << std::endl;
                    }
                    break;
                }
                default:
                    break;
            }
        }

        if(!run.load()) {
            close(new_socket);
        }
    }          

    void removeNode(int nodeId) {
        int masterNode_socket = nodeList.getSocketToMasterNode();
        std::cout << "[Server] masterNode_socket: " << masterNode_socket << std::endl;
        if (masterNode_socket != -1 && nodeList.isNodeInMesh(nodeId)) {
            NodeData nodeData = {0};
            nodeData.nodeId = nodeId;
            nodeData.port = PORT;
            char ipAddress[256];
            ipUtils.getIPAddress(ipAddress, sizeof(ipAddress));
            strcpy(nodeData.ipAddress, ipAddress);
            strcpy(nodeData.action, actionToString(ActionType::REMOVE_NODE).c_str());
            std::cout << "[Server] droneId: " << nodeData.nodeId << ", port: " << nodeData.port << ", ipAddress: " << nodeData.ipAddress
                << ", action: " << nodeData.action << std::endl;
            send(masterNode_socket, &nodeData, sizeof(nodeData), 0);
        } else {
            std::cerr << "[Server] Could not remove node. Please control if the node is in the mesh network and that the master node socket is stored" << std::endl;
        }
    }

    void handleConnections() {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int new_socket;

        while (run.load()) {
            

            if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
                perror("new connection failed");
                exit(EXIT_FAILURE);
            }

            threads.emplace_back([&] {
                handleConnection(new_socket);
            });
        }

        if(!run.load()) {
            stop();
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
