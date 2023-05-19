#include "Node.h"

Node::Node() {}

Node::Node(int nodeId, int port, int ipAddress)
        : nodeId(nodeId), port(port), ipAddress(ipAddress), xPosition(0), yPosition(0), priority(Priority::NONE) {
}

Node::Node(int nodeId, int socket)
        : nodeId(nodeId), port(0), ipAddress(0), xPosition(0), yPosition(0), socket(socket), priority(Priority::NONE) {
}

Node::Node(int nodeId, int port, int ipAddress, int xPosition, int yPosition, int socket, Priority priority)
        : nodeId(nodeId), port(port), ipAddress(ipAddress), xPosition(xPosition), yPosition(yPosition),
        socket(socket), priority(priority) {
}

Node::Node(const Node& other)
        : nodeId(other.nodeId), port(other.port), ipAddress(other.ipAddress), xPosition(other.xPosition),
          yPosition(other.yPosition), socket(other.socket), priority(other.priority) {
}

Node::Node(const NodeData& nodeData) 
        : nodeId(nodeData.nodeId), port(nodeData.port), ipAddress(0), xPosition(0), yPosition(0), priority(Priority::NONE) {
}

int Node::getNodeId() const {
    return nodeId;
}

int Node::getPort() const {
    return port;
}

int Node::getIpAddress() const {
    return ipAddress;
}

int Node::getXPosition() const {
    return xPosition;
}

void Node::setXPosition(int x) {
    xPosition = x;
}

int Node::getYPosition() const {
    return yPosition;
}

void Node::setYPosition(int y) {
    yPosition = y;
}

int Node::getSocket() const {
    return socket;
}

Priority Node::getPriority() const {
    return priority;
}

void Node::setPriority(Priority p) {
    priority = p;
}
