//
// Created by HansMagne Asheim on 16/05/2023.
//

#ifndef MESH_NETWORK_NODELIST_HPP
#define MESH_NETWORK_NODELIST_HPP

#include <unordered_map>
#include <iostream>

#include "Node.hpp"
#include "DoubleLinkedList.hpp"

class NodeList {
private:
    std::unordered_map<int, Node*> nodes;
    DoubleLinkedList meshNetwork;
    bool insertRight = false;
    int meshSize;
    int socketToMasterNode = -1;
    void resetNode(Node*);
    void copyNodeInformation(Node*, Node*);
    void toString(Node*);
    void updatePosition();
    Node* getHighPriority();
    /*
    * Check if node is in NodeList or MeshNetwork
    * If bool is false then it is controlled if the nodeId is in NodeList.
    * If bool is true then it is controlled if the nodeId is in MeshNetwork.
    */
    bool isNode(const int, bool);

public:
    NodeList(int meshSize);

    Node* addNode(const Node&);
    Node* addNode(const NodeData&);
    Node* getNode(const int);
    //void editNode(Node*);
    int getSize() const;

    void setSocketToMasterNode(const int);
    int getSocketToMasterNode() const;
    void setPriority(Node*);
    Node* addNodeToMesh(Node&);
    void replaceNode(const int, const int);
    bool isNodeInMesh(const int);
    bool isNodeInList(const int);
    std::unordered_map<int, Node*> getNodesWithPriority(Priority);

    /*
    * Getter for the connected inner node
    * If the node is master node is null returned, if not is the connected inner node returned
    */
    Node* getConnectedInnerNode(Node*);
    
    /*
    * Controls if the mesh has reach it's maximal size
    * Returnes true if the mesh has reaced it's size, otherwise false
    */
    bool isMeshFull();
};
#endif //MESH_NETWORK_NODELIST_HPP
