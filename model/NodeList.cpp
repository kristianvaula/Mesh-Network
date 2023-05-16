//
// Created by HansMagne Asheim on 16/05/2023.
//
#include "NodeList.h"

NodeList::NodeList() {}

void NodeList::addNode(const Node& node) {
    nodes.insert(std::make_pair(node.getNodeId(), node));
}

void NodeList::addNode(const NodeData& nodeData) {
    Node node(nodeData);
    nodes.insert(std::make_pair(node.getNodeId(), node));
}

void NodeList::editNode(const Node& node) {
    if (nodes.count(node.getNodeId()) > 0) {
        nodes.insert(std::make_pair(node.getNodeId(), node));
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

bool NodeList::isNodeInMesh() const {
    return nodes.size() > getNodesWithPriority(Priority::NONE).size();
}
