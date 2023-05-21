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
    void resetNode(Node*);//new method
    void copyNodeInformation(Node*, Node*);//new method
    void toString(Node*);//new method
    void updatePosition();//new method
    Node* getHighPriority();//new method

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
    void replaceNode(const int, const int);//new method
    bool isNodeInMesh(const int);
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
