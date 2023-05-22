//
// Created by HansMagne Asheim on 16/05/2023.
//

#ifndef MESH_NETWORK_NODE_HPP
#define MESH_NETWORK_NODE_HPP

#include <string.h>

#include "enums/Priority.hpp"
#include "dto/SocketData.hpp"

class Node {
private:
    int nodeId;
    int port;
    std::string ipAddress;
    int xPosition;
    int yPosition;
    Priority priority;

public:
    Node* prev = nullptr;
    Node* next = nullptr;

    Node();
    Node(int nodeId, int port, std::string ipAddress);
    Node(int nodeId, int port, std::string ipAddress, int xPosition, int yPosition, Priority priority);
    Node(const Node& other);
    Node(const NodeData& nodeData);

    int getNodeId() const;
    int getPort() const;
    std::string getIpAddress() const;
    int getXPosition() const;
    void setXPosition(int x);
    int getYPosition() const;
    void setYPosition(int y);
    Priority getPriority() const;
    std::string getPriorityName();
    void setPriority(Priority p);
};

#endif //MESH_NETWORK_NODE_HPP
