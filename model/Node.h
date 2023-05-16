//
// Created by HansMagne Asheim on 16/05/2023.
//

#ifndef MESH_NETWORK_NODE_H
#define MESH_NETWORK_NODE_H

#include "enums/Priority.cpp"
#include "dto/SocketData.cpp"

class Node {
private:
    int nodeId;
    int port;
    int ipAddress;
    int xPosition;
    int yPosition;
    Priority priority;

public:
    Node(int nodeId, int port, int ipAddress);
    Node(int nodeId, int port, int ipAddress, int xPosition, int yPosition, Priority priority);
    Node(const Node& other);
    Node(const NodeData& nodeData);

    int getNodeId() const;
    int getPort() const;
    int getIpAddress() const;
    int getXPosition() const;
    void setXPosition(int x);
    int getYPosition() const;
    void setYPosition(int y);
    Priority getPriority() const;
    void setPriority(Priority p);
};

#endif //MESH_NETWORK_NODE_H
