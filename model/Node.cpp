#include "Node.hpp"

Node::Node() {}

Node::Node(int nodeId, int port, std::string ipAddress)
        : nodeId(nodeId), port(port), ipAddress(ipAddress), xPosition(0), yPosition(0), priority(Priority::NONE) {
}

Node::Node(int nodeId, int port, std::string ipAddress, int xPosition, int yPosition, Priority priority)
        : nodeId(nodeId), port(port), ipAddress(ipAddress), xPosition(xPosition), yPosition(yPosition),
        priority(priority) {
}

Node::Node(const Node& other)
        : nodeId(other.nodeId), port(other.port), ipAddress(other.ipAddress), xPosition(other.xPosition),
          yPosition(other.yPosition), priority(other.priority) {
}

Node::Node(const NodeData& nodeData) 
        : nodeId(nodeData.nodeId), port(nodeData.port), ipAddress(nodeData.ipAddress), xPosition(0), yPosition(0), priority(Priority::NONE) {
}

int Node::getNodeId() const {
    return nodeId;
}

int Node::getPort() const {
    return port;
}

std::string Node::getIpAddress() const {
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

Priority Node::getPriority() const {
    return priority;
}

std::string Node::getPriorityName() {
    if (priority == Priority::HIGH) return "HIGH";
    else if (priority == Priority::MEDIUM) return "MEDIUM";
    else if (priority == Priority::LOW) return "LOW";
    else return "NONE";
}

void Node::setPriority(Priority p) {
    priority = p;
}
