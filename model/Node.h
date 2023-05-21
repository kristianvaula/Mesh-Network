//
// Created by HansMagne Asheim on 16/05/2023.
//

#ifndef MESH_NETWORK_NODE_H
#define MESH_NETWORK_NODE_H

#include "/nettverksprog/mesh-network/model/enums/Priority.h"
#include "/nettverksprog/mesh-network/model/dto/SocketData.h"

class Node {
private:
    int nodeId;
    int port;
    std::string ipAddress;
    int xPosition;
    int yPosition;
    Priority priority;

public:
    Node* prev;
    Node* next;

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
    void setPriority(Priority p);
};

#endif //MESH_NETWORK_NODE_H
