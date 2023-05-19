//
// Created by HansMagne Asheim on 16/05/2023.
//
#include <iostream>

#include "/nettverksprog/mesh-network/model/NodeList.h"

NodeList::NodeList(int meshSize) 
        : meshSize(meshSize) {
}

void NodeList::addNode(const Node& node) {
    nodes.insert(std::make_pair(node.getNodeId(), node));
}

void NodeList::addNode(const NodeData& nodeData) {
    Node node(nodeData);
    nodes.insert(std::make_pair(node.getNodeId(), node));
}

Node NodeList::getNode(const int nodeId) {
    return nodes[nodeId];
}

void NodeList::editNode(const Node& node) {
    if (nodes.count(node.getNodeId()) > 0) {
        nodes[node.getNodeId()] = node;
    }
}

int NodeList::getSize() const {
    return nodes.size();
}

std::unordered_map<int, Node> NodeList::getNodesWithPriority(Priority priority) const {
    std::unordered_map<int, Node> priorityNodes;

    for (const auto& entry : nodes) {
        const Node& node = entry.second;

        if (node.getPriority() == priority) {
            priorityNodes.insert(std::make_pair(node.getNodeId(), node));
        }
    }

    return priorityNodes;
}

int NodeList::getSocketToMasterNode() const {
     if (!nodes.empty()) {
        auto priorityNodes = getNodesWithPriority(Priority::HIGH);
        if (!priorityNodes.empty()) {
            const Node& node = priorityNodes.begin()->second;
            return node.getSocket();
        }
    }
    
    return -1;
}

Node NodeList::addNodeToMesh(const NodeData& nodeData) {//hardcoded if there is only 5 positions 
    int size = getSize();
    Priority priority;
    int location;
    Node node(nodeData);

    if (size == 0) {
        node.setPriority(Priority::HIGH);
        node.setXPosition(0);
        std::cout << "Priority high for nodeId: " << node.getNodeId() << std::endl;
        std::cout << "Location: " << node.getXPosition() << std::endl; 
    } else if (size == 1) {
        node.setPriority(Priority::MEDIUM);
        node.setXPosition(1);
        std::cout << "Priority medium for nodeId: " << node.getNodeId() << std::endl;
        std::cout << "Location: " << node.getXPosition() << std::endl;
    } else if (size == 2) {
        node.setPriority(Priority::MEDIUM);
        node.setXPosition(-1);
        std::cout << "Priority medium for nodeId: " << node.getNodeId() << std::endl;
        std::cout << "Location: " << node.getXPosition() << std::endl;
    } else if (size == 3) {
        node.setPriority(Priority::LOW);
        node.setXPosition(2);
        std::cout << "Priority low for nodeId: " << node.getNodeId() << std::endl;
        std::cout << "Location: " << node.getXPosition() << std::endl;
    } else {
        node.setPriority(Priority::LOW);
        node.setXPosition(-2);
        std::cout << "Priority low for nodeId: " << node.getNodeId() << std::endl;
        std::cout << "Location: " << node.getXPosition() << std::endl;
    }
    addNode(node);
    return node;
}

bool NodeList::isMeshFull() const {
    return nodes.size() - getNodesWithPriority(Priority::NONE).size() == meshSize;
}
