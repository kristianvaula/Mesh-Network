//
// Created by HansMagne Asheim on 16/05/2023.
//
#include "NodeList.hpp"

NodeList::NodeList(int meshSize) 
        : meshSize(meshSize) {
}

Node* NodeList::addNode(const Node& node) {
    auto insertionResult = nodes.insert({node.getNodeId(), new Node(node)});
    if (insertionResult.second) {
        return insertionResult.first->second;
    }
    return nullptr;
}

Node* NodeList::addNode(const NodeData& nodeData) {
    Node node(nodeData);
    return addNode(node);
}

Node* NodeList::getNode(const int nodeId) {
    auto iterator = nodes.find(nodeId);
    if (iterator != nodes.end()) {
        return iterator->second;
    }
    return nullptr;
}

int NodeList::getSize() const {
    return nodes.size();
}

std::unordered_map<int, Node*> NodeList::getNodesWithPriority(Priority priority) {
    std::unordered_map<int, Node*> priorityNodes;

    for (const auto& entry : nodes) {
        int nodeId = entry.first;
        Node* node = entry.second;

        if (node->getPriority() == priority) {
            priorityNodes[nodeId] = node;
        }
    }

    return priorityNodes;
}

void NodeList::setSocketToMasterNode(const int socket) {
    this->socketToMasterNode = socket;
}

int NodeList::getSocketToMasterNode() const {
     return this->socketToMasterNode;
}

void NodeList::setPriority(Node* node) {
    if (node->getXPosition() == 0) {
        node->setPriority(Priority::HIGH);
        std::cout << "[NodeList] Sets HIGH priority for nodeId: " << node->getNodeId() << std::endl;
    } else if (node->getXPosition() == 1 || node->getXPosition() == -1) {
        node->setPriority(Priority::MEDIUM);
        std::cout << "[NodeList] Sets MEDIUM priority for nodeId: " << node->getNodeId() << std::endl;
    } else {
        node->setPriority(Priority::LOW);
        std::cout << "[NodeList] Sets LOW priority for nodeId: " << node->getNodeId() << std::endl;
    }
}

Node* NodeList::addNodeToMesh(Node& node) {
    Node* nodeListItem = addNode(node);
    if(this->insertRight) {
        this->meshNetwork.insertEnd(nodeListItem);
        if(nodeListItem->prev != nullptr) {
            nodeListItem->setXPosition(nodeListItem->prev->getXPosition() + 1);
        } 
        this->insertRight = false;
    } else {
        this->meshNetwork.insertFront(nodeListItem);
        if(nodeListItem->next != nullptr) {
            nodeListItem->setXPosition(nodeListItem->next->getXPosition() - 1);
        } else {//master node
            nodeListItem->setXPosition(0);
        }
        this->insertRight = true;
    }
    std::cout << "[NodeList] Sets xPosition to nodeId: " << nodeListItem->getNodeId() << " to be xpos: " << nodeListItem->getXPosition() << std::endl;
    this->setPriority(nodeListItem);
    return nodeListItem;
}

void NodeList::resetNode(Node* node) {
    node->setPriority(Priority::NONE);
    node->setXPosition(0);
    //reset of prev and next is done while replacing node in double linked list
} 

void NodeList::toString(Node* node) {
    std::cout << "nodeId: " << node->getNodeId() << ", xPosition: " << node->getXPosition() << std::endl;
    if (node->prev) {
        std::cout  << "prev nodeId: " << node->prev->getNodeId() << std::endl;
    } 
    if (node->next) {
        std::cout  << "next nodeId: " << node->next->getNodeId() << std::endl;
    }
}

Node* NodeList::getHighPriority() {
    for (const auto& entry : nodes) {
        int nodeId = entry.first;
        Node* node = entry.second;

        if (node->getPriority() == Priority::HIGH) {
            return node;
        }
    }
    return nullptr;
}

void NodeList::updatePosition() {
    if (nodes.size() == 0) return;
    Node* highPriorityNode = getHighPriority();
    if (highPriorityNode != nullptr) {
        Node* currentNode = highPriorityNode;
        while(currentNode->next) {
            currentNode->next->setXPosition(currentNode->getXPosition() + 1);
            setPriority(currentNode->next);
            std::cout << "[NodeList] updating positon to nodeId: " << currentNode->getNodeId() << " the xPosition is now: " << currentNode->getXPosition() << std::endl;
            currentNode = currentNode->next;
        }
        std::cout << "[NodeList] updating positon to nodeId: " << currentNode->getNodeId() << " the xPosition is now: " << currentNode->getXPosition() << std::endl;
        currentNode = highPriorityNode;
        while(currentNode->prev) {
            currentNode->prev->setXPosition(currentNode->getXPosition() - 1);
            setPriority(currentNode->prev);
            std::cout << "[NodeList] updating positon to nodeId: " << currentNode->getNodeId() << " the xPosition is now: " << currentNode->getXPosition() << std::endl;
            currentNode = currentNode->prev;
        }
        std::cout << "[NodeList] updating positon to nodeId: " << currentNode->getNodeId() << " the xPosition is now: " << currentNode->getXPosition() << std::endl;
    }
}

void NodeList::copyNodeInformation(Node* node, Node* replacementNode) {
    meshNetwork.replaceNode(node, replacementNode);
    replacementNode->setPriority(node->getPriority());
    replacementNode->setXPosition(node->getXPosition());
}

void NodeList::replaceNode(const int nodeId, const int replacementNodeId) {
    Node* node = getNode(nodeId);
    Node* replacementNode = getNode(replacementNodeId);
    if (node != nullptr && replacementNode != nullptr) {
        std::cout << "[NodeList] Replacing nodeId: " << nodeId << " with replacementNodeId " << replacementNodeId << std:: endl;
        copyNodeInformation(node, replacementNode);
        resetNode(node);
        updatePosition();
    } else if (node != nullptr) {
        std::cout << "[NodeList] Deleting nodeId: " << nodeId << std:: endl;
        meshNetwork.removeNode(node);
        resetNode(node);
        updatePosition();
    } else {
        std::cerr << "[NodeList] One or both node ids does not exist in the register" << std::endl;
    }
}

bool NodeList::isNode(const int nodeId, bool inMesh) {
    auto iterator = nodes.find(nodeId);
    if (iterator != nodes.end()) {
        Node* nodeInNodeList = iterator->second;
        return inMesh ? nodeInNodeList->getPriority() != Priority::NONE : true;
    }
    return false;
}

bool NodeList::isNodeInList(const int nodeId) {
    return isNode(nodeId, false);
}

bool NodeList::isNodeInMesh(const int nodeId) {
    return isNode(nodeId, true);
}

Node* NodeList::getConnectedInnerNode(Node* node) {
    if(node->getXPosition() == 0) {
        return nullptr;
    } else if (node->getXPosition() > 0) {
        std::cout << "[NodeList] The connected inner node for nodeId " << node->getNodeId() << " is nodeId " << node->prev->getNodeId() << std::endl;
        return node->prev;
    }
    std::cout << "[NodeList] The connected inner node for nodeId " << node->getNodeId() << " is nodeId " <<  node->next->getNodeId() << std::endl;
    return node->next;
}

bool NodeList::isMeshFull() {
    return nodes.size() - getNodesWithPriority(Priority::NONE).size() == meshSize;
}
