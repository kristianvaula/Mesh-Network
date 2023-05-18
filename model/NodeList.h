//
// Created by HansMagne Asheim on 16/05/2023.
//

#ifndef MESH_NETWORK_NODELIST_H
#define MESH_NETWORK_NODELIST_H

#include <unordered_map>
#include "/nettverksprog/mesh-network/model/Node.h"

class NodeList {
private:
    std::unordered_map<int, Node> nodes;

public:
    NodeList();

    void addNode(const Node& node);
    void addNode(const NodeData& nodeData);
    void editNode(const Node& node);
    int getSize() const;
    std::unordered_map<int, Node> getNodesWithPriority(Priority priority) const;
    bool isNodeInMesh() const;
};
#endif //MESH_NETWORK_NODELIST_H
