//
// Created by HansMagne Asheim on 16/05/2023.
//
#include "enums/Priority.cpp"

class Node {
private:
    int nodeId;
    int port;
    int ipAddress;
    int xPosition;
    int yPosition;
    Priority priority;

public:
    Node(int nodeId, int port, int ipAddress)
        : nodeId(nodeId), port(port), ipAddress(ipAddress), xPosition(0), yPosition(0), priority(Priority::NONE) {}

    Node(int nodeId, int port, int ipAddress, int xPosition, int yPosition, Priority priority)
        : nodeId(nodeId), port(port), ipAddress(ipAddress), xPosition(xPosition), yPosition(yPosition),
        priority(priority) {}

    Node(const Node& other)
        : nodeId(other.nodeId), port(other.port), ipAddress(other.ipAddress), xPosition(other.xPosition),
        yPosition(other.yPosition), priority(other.priority) {}

    Node(const NodeData& nodeData) : nodeId(nodeData.nodeId), port(nodeData.port) {}

    int getNodeId() const {
        return nodeId;
    }

    int getPort() const {
        return port;
    }

    int getIpAddress() const {
        return ipAddress;
    }

    int getXPosition() const {
        return xPosition;
    }

    void setXPosition(int x) {
        xPosition = x;
    }

    int getYPosition() const {
        return yPosition;
    }

    void setYPosition(int y) {
        yPosition = y;
    }

    Priority getPriority() const {
        return priority;
    }

    void setPriority(Priority p) {
        priority = p;
    }
};