//
// Created by HansMagne Asheim on 16/05/2023.
//

#ifndef MESH_NETWORK_NODELIST_H
#define MESH_NETWORK_NODELIST_H

#include <unordered_map>
#include "/nettverksprog/mesh-network/model/Node.h"
#include "DoubleLinkedList.cpp"

class NodeList {
private:
    std::unordered_map<int, Node> nodes;
    DoubleLinkedList meshNetwork;
    bool insertRight = false;
    int meshSize;
    int socketToMasterNode = -1;

public:
    NodeList(int meshSize);

    void addNode(const Node&);
    void addNode(const NodeData&);
    Node getNode(const int);
    void editNode(const Node&);
    int getSize() const;
    void setSocketToMasterNode(const int);
    int getSocketToMasterNode() const;
    void setPriority(Node&);
    Node addNodeToMesh(const NodeData&);
    std::unordered_map<int, Node> getNodesWithPriority(Priority) const;

    /*
    * Getter for the connected inner node
    * If the node is master node is null returned, if not is the connected inner node returned
    */
    Node* getConnectedInnerNode(const Node&);
    
    /*
    * Controls if the mesh has reach it's maximal size
    * Returnes true if the mesh has reaced it's size, otherwise false
    */
    bool isMeshFull() const;
};
#endif //MESH_NETWORK_NODELIST_H
