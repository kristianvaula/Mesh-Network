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
#include <cerrno>
#include <sys/types.h>
#include <sys/time.h> 
#include <sys/select.h>

#include "../model/enums/ActionType.hpp"
#include "../model/NodeList.hpp"
#include "IpUtils.hpp"

#define PORT 8083

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
        cleanup();
        std::cout << "[ Server ] Shutting down" << std::endl;
        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
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
        std::cout << "[ Server ] Server is running on port: " << PORT << std::endl
            << "[ Server ] 'q' - Exit" << std::endl
            << "[ Server ] 'r' - Remove node from mesh network" << std::endl
            << "[ Server ] Choose an option:" << std::endl;
    }

    void handleServerInput() {
        bool serverInput = true;
        char x;

        while (serverInput) {
            displayMenu();
            std::cin >> x;
            if (x == 'q') {
                serverInput = false;
                run.store(false);
                break;
            } else if (x == 'r') {
                int nodeId;
                std::cout << "[ Server ] Type in the nodeId to the node you want to remove" << std::endl;
                std::cin >> nodeId;
                std::cout << "[ Server ] Recived request to remove nodeId: " << nodeId << std::endl;
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

        //Set timeout
        timeval timeout; 
        timeout.tv_sec = 3; 
        timeout.tv_usec = 0; 

        while (clientConnection && run.load()) {
            NodeData nodeData = {0};

            //Confgure timeout
            timeval timeoutCopy = timeout; 

            fd_set readfds; 
            FD_ZERO(&readfds); 
            FD_SET(new_socket, &readfds); 

            int selectResult = select(new_socket + 1, &readfds, nullptr, nullptr, &timeoutCopy); 
            if (selectResult == -1) {
                std::cerr << "[ ERROR ] Error in select" << std::endl; 
                break; 
            }
            else if (selectResult == 0) {
                continue; 
            }

            if (FD_ISSET(new_socket, &readfds)) {  
                valread = recv(new_socket, &nodeData, sizeof(nodeData), 0);

                if (valread <= 0) {
                    clientConnection = false;
                    std::cout << "[ Server: clientThread ] Closing client conection" << std::endl;
                    close(new_socket);
                    return;
                } 
                
                std::cout << "[ Server: clientThread ] Recieve a message from node id: " << nodeData.nodeId << ", action: " << nodeData.action << std::endl;
                
                ActionType action = actionFromString(nodeData.action);
                switch(action) {
                    case ActionType::HELLO: {
                        if (nodeList.isNodeInList(nodeData.nodeId)) {
                            std::cerr << "[ Server: clientThread ] nodeId: " << nodeData.nodeId << " is in pool" << std::endl;
                            break;
                        }

                        Node node(nodeData);
                        {//aquire lock
                            std::unique_lock<std::mutex> lock(this->nodeList_mutex);
                            if (!nodeList.isMeshFull()) {
                                std::cout << "[ Server: clientThread ] Adding nodeId " << nodeData.nodeId << " to mesh" << std::endl;
                                nodeList.addNodeToMesh(node);
                                nodeList.printMeshNetwork(); 
                            } else {
                                std::cout << "[ Server: clientThread ] Adding nodeId " << nodeData.nodeId << " to pool" << std::endl;
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
                        try{
                            int replacementNodeId = std::stoi(arg); 
                            {//aquire lock
                                std::unique_lock<std::mutex> lock(this->nodeList_mutex);
                                nodeList.replaceNode(nodeData.nodeId, replacementNodeId);
                                nodeList.printMeshNetwork(); 
                            }//release lock
                        }
                        catch(std::invalid_argument e){
                            std::cerr << "[ Server: clientThread ] Received invalid positon" << std::endl; 
                        }
                        catch(std::out_of_range e){
                            std::cerr << "[ Server: clientThread ] Received invalid positon" << std::endl;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
        std::cout << "[ Server: clientThread ] client thread closed" << std::endl;
        close(new_socket);
    }          

    void removeNode(int nodeId) {
        int masterNode_socket = nodeList.getSocketToMasterNode();
        if (masterNode_socket != -1 && nodeList.isNodeInMesh(nodeId)) {
            NodeData nodeData = {0};
            nodeData.nodeId = nodeId;
            nodeData.port = PORT;
            char ipAddress[256];
            ipUtils.getIPAddress(ipAddress, sizeof(ipAddress));
            strcpy(nodeData.ipAddress, ipAddress);
            strcpy(nodeData.action, actionToString(ActionType::REMOVE_NODE).c_str());
            send(masterNode_socket, &nodeData, sizeof(nodeData), 0);
        } else {
            std::cerr << "[ Error ] Could not remove node. Please control if the node is in the mesh network" << std::endl;
        }
    }

    void handleConnections() {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int new_socket;

        //Set timeout
        timeval timeout; 
        timeout.tv_sec = 3; 
        timeout.tv_usec = 0; 

        while (run.load()) {
            //Confgure timeout
            timeval timeoutCopy = timeout; 

            fd_set readfds; 
            FD_ZERO(&readfds); 
            FD_SET(server_fd, &readfds); 
            
            int selectResult = select(server_fd + 1, &readfds, nullptr, nullptr, &timeoutCopy); 
            if (selectResult == -1) {
                std::cerr << "[ ERROR ] Error in select" << std::endl; 
                break; 
            }
            else if (selectResult == 0) {
                continue; 
            }

            if (FD_ISSET(server_fd, &readfds)) {
                if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
                    perror("new connection failed");
                    exit(EXIT_FAILURE);
                }

                threads.emplace_back([&] {
                    handleConnection(new_socket);
                });
            };
        }

        if(!run.load()) {
            stop();
        }
    }

    void cleanup() {
        for (auto& thread : threads) {
            thread.join();
        }
        if (inputThread.joinable()) {
            inputThread.join();
        }
        std::cout << "[ Server ] Server joined" << std::endl;
    }
};

int main(int argc, char const* argv[]) {
    Server server;
    server.start();

    return 0;
}
