//
// Created by HansMagne Asheim on 16/05/2023.
//

#ifndef MESH_NETWORK_NODE_H
#define MESH_NETWORK_NODE_H

#include "/nettverksprog/mesh-network/model/enums/Priority.h"
#include "/nettverksprog/mesh-network/model/dto/SocketData.h"

//TODO: It is not neccessary to have both port, ipAddress and socket
class Node {
private:
    int nodeId;
    int port;
    int ipAddress;
    int xPosition;
    int yPosition;
    int socket;
    Priority priority;

public:
    Node();
    Node(int nodeId, int port, int ipAddress);
    Node(int nodeId, int socket);
    Node(int nodeId, int port, int ipAddress, int xPosition, int yPosition, int socket, Priority priority);
    Node(const Node& other);
    Node(const NodeData& nodeData);

    int getNodeId() const;
    int getPort() const;
    int getIpAddress() const;
    int getXPosition() const;
    void setXPosition(int x);
    int getYPosition() const;
    void setYPosition(int y);
    int getSocket() const;
    Priority getPriority() const;
    void setPriority(Priority p);
};

#endif //MESH_NETWORK_NODE_H
