#ifndef MESH_NETWORK_DOUBLE_LINKED_LIST_HPP
#define MESH_NETWORK_DOUBLE_LINKED_LIST_HPP

#include "Node.hpp"
#include <ostream>
#include <iostream>

class DoubleLinkedList {
private:
    Node* head;
    Node* tail;


public:
    DoubleLinkedList();
    void insertFront(Node*);
    void insertEnd(Node*);
    void replaceNode(Node*, Node*);
    void removeNode(Node*);
    Node* getHead();
};

#endif //MESH_NETWORK_DOUBLE_LINKED_LIST_HPP