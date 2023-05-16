//
// Created by HansMagne Asheim on 16/05/2023.
//
#include <unordered_map>

#include "Node.cpp"

class NodeList {
private:
    std::unordered_map<int, Node> nodes;

public:
    NodeList() {}

    void addNode(const Node& node) {
        nodes.insert(std::make_pair(node.getNodeId(), node));
    }

    void addNode(const NodeData& nodeData) {
        Node node(nodeData);
        nodes.insert(std::make_pair(node.getNodeId(), node));
    }

    void editNode(const Node& node) {
        if (nodes.count(node.getNodeId()) > 0) {
            nodes.insert(std::make_pair(node.getNodeId(), node));
        }
    }

    int getSize() {
        return nodes.size();
    }

    std::unordered_map<int, Node> getNodesWithPriority(Priority priority) const {
        std::unordered_map<int, Node> priorityNodes;

        for (const auto& entry : nodes) {
            const Node& node = entry.second;

            if (node.getPriority() == priority) {
                priorityNodes.insert(std::make_pair(node.getNodeId(), node));
            }
        }

        return priorityNodes;
    }

    bool isNodeInMesh() const {
        return nodes.size() > getNodesWithPriority(Priority::NONE).size();
    }
};
