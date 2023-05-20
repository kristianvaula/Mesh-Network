//
// Created by HansMagne Asheim on 16/05/2023.
//
#include <iostream>

#include "/nettverksprog/mesh-network/model/NodeList.h"

NodeList::NodeList(int meshSize) 
        : meshSize(meshSize) {
}

Node NodeList::addNode(const Node& node) {
    auto insertionResult = nodes.insert(std::make_pair(node.getNodeId(), node));
    if (insertionResult.second) {
        return insertionResult.first->second;
    }
    return Node();
}

Node NodeList::addNode(const NodeData& nodeData) {
    Node node(nodeData);
    return addNode(node);
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

void NodeList::setSocketToMasterNode(const int socket) {
    socketToMasterNode = socket;
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

Node NodeList::addNodeToMesh(Node& node) {
    if(this->insertRight) {
        this->meshNetwork.insertEnd(&node);
        if(node.prev != nullptr) {
            node.setXPosition(node.prev->getXPosition() + 1);
            std::cout << "Previous nodeid: " << node.prev->getNodeId() << std::endl
            << "My nodeId " << node.getNodeId() << std::endl;;
            std::cout << "Sets xPostion " << node.prev->getXPosition() + 1 << std::endl;
        } 
        this->insertRight = false;
    } else {
        this->meshNetwork.insertFront(&node);
        if(node.next != nullptr) {
            node.setXPosition(node.next->getXPosition() - 1);
            std::cout << "Next nodeid: " << node.next->getNodeId() << std::endl
            << "My nodeId " << node.getNodeId() << std::endl;
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
//new method
void NodeList::resetNode(Node& node) {
    node.setPriority(Priority::NONE);
    node.setXPosition(0);
    node.prev = nullptr;
    node.next = nullptr;
} 

//new method
void NodeList::toString(Node& node) {
    std::cout << "nodeId: " << node.getNodeId() << ", xPosition: " << node.getXPosition() << std::endl;
    if (node.prev) {
        std::cout  << "prev nodeId: " << node.prev->getNodeId() << std::endl;
    } 
    if (node.next) {
        std::cout  << "next nodeId: " << node.next->getNodeId() << std::endl;
    }
}

//new method
void NodeList::copyNodeInformation(Node& node, Node& replacementNode) {
    toString(node);
    toString(replacementNode);
    meshNetwork.replaceNode(&node, &replacementNode);
    replacementNode.setPriority(node.getPriority());
    replacementNode.setXPosition(node.getXPosition());
    toString(node);
    toString(replacementNode);
}

//new method
void NodeList::replaceNode(const int nodeId, const int replacementNodeId) {
    std::cout << "Replcaing nodeId: " << nodeId << " with replacementNodeId " << replacementNodeId << std:: endl;
    Node node = nodes[nodeId];
    std::cout  << "REPLACE NODE prev nodeId: " << node.prev->getNodeId() << std::endl;
    std::cout  << "REPLACE NODE next nodeId: " << node.next->getNodeId() << std::endl;
    Node replacementNode = nodes[replacementNodeId];

    copyNodeInformation(node, replacementNode);
    resetNode(node);
}

bool NodeList::isNodeInMesh(const int nodeId) {
    auto iterator = nodes.find(nodeId);
    if (iterator != nodes.end()) {
        Node& nodeInNodeList = iterator->second;
        return nodeInNodeList.getPriority() != Priority::NONE;
    }
    return false;
}

Node* NodeList::getConnectedInnerNode(const Node& node) {
    if(node.getXPosition() == 0) {
        return nullptr;
    } else if (node.getXPosition() > 0) {
        return node.prev;
    }
    return node.next;
}

bool NodeList::isMeshFull() const {
    return nodes.size() - getNodesWithPriority(Priority::NONE).size() == meshSize;
}
