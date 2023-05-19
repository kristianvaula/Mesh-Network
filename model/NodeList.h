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
    int meshSize;

public:
    NodeList(int meshSize);

    void addNode(const Node& node);
    void addNode(const NodeData& nodeData);
    Node getNode(const int nodeId);
    void editNode(const Node& node);
    int getSize() const;
    int getSocketToMasterNode() const;
    Node addNodeToMesh(const NodeData& nodeData);
    std::unordered_map<int, Node> getNodesWithPriority(Priority priority) const;
    
    /*
    * Controls if the mesh has reach it's maximal size
    * Returnes true if the mesh has reaced it's size, otherwise false
    */
    bool isMeshFull() const;
};
#endif //MESH_NETWORK_NODELIST_H
