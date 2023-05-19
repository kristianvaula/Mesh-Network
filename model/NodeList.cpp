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
     return this->socketToMasterNode;
}

void NodeList::setPriority(Node& node) {
    if (node.getXPosition() == 0) {
        node.setPriority(Priority::HIGH);
        std::cout << "Sets HIGH priority" << std::endl;
    } else if (node.getXPosition() == 1 || node.getXPosition() == -1) {
        node.setPriority(Priority::MEDIUM);
        std::cout << "Sets MEDIUM priority" << std::endl;
    } else {
        node.setPriority(Priority::LOW);
        std::cout << "Sets LOW priority" << std::endl;
    }
}

Node NodeList::addNodeToMesh(const NodeData& nodeData) {//hardcoded if there is only 5 positions 
    int size = getSize();
    Priority priority;
    int location;
    Node node(nodeData);

    if(this->insertRight) {
        this->meshNetwork.insertEnd(&node);
        if(node.prev != nullptr) {
            node.setXPosition(node.prev->getXPosition() + 1);;
            std::cout << "Sets xPostion " << node.prev->getXPosition() + 1 << std::endl;
        } 
        this->insertRight = false;
    } else {
        this->meshNetwork.insertFront(&node);
        if(node.next != nullptr) {
            node.setXPosition(node.next->getXPosition() - 1);
            std::cout << "Sets xPostion " << node.next->getXPosition() - 1 << std::endl;
        } else {//master node
            node.setXPosition(0);
            std::cout << "Sets xPostion 0" << std::endl;
        }
        this->insertRight = true;
    }
    this->setPriority(node);

    addNode(node);
    return node;
}

bool NodeList::isMeshFull() const {
    return nodes.size() - getNodesWithPriority(Priority::NONE).size() == meshSize;
}
