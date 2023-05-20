#include "Node.h"
#include <ostream>
#include <iostream>

class DoubleLinkedList {
private:
    Node* head;
    Node* tail;

public:
    DoubleLinkedList() : head(nullptr), tail(nullptr) {}

    void insertFront(Node* node) {
        if (head == nullptr) {
            head = tail = node;
        } else {
            node->next = head;
            head->prev = node;
            head = node;
        }
    }

    void insertEnd(Node* node) {
        if (tail == nullptr) {
            head = tail = node;
        } else {
            node->prev = tail;
            tail->next = node;
            tail = node;
        }
    }
    //new method
    void replaceNode(Node* node, Node* replacementNode) {
        if (node == nullptr || replacementNode == nullptr) return;
        if (head == nullptr || tail == nullptr) return;
    
        if (node == head) {
            replacementNode->next = head->next;
            if (head->next) {
                head->next->prev = replacementNode;
            }
            head = replacementNode;
            return;
        }
    
        if (node == tail) {
            replacementNode->prev = tail->prev;
            if (tail->prev) {
                tail->prev->next = replacementNode;
            }
            tail = replacementNode;
            return;
        }

        if (node->prev) {
            node->prev->next = replacementNode;
        }
        if (replacementNode->prev) {
            replacementNode->prev->next = replacementNode;
        }
        if (node->next) {
            node->next->prev = replacementNode;
        }
        if (replacementNode->next) {
            replacementNode->next->prev = replacementNode;
        }
    
        replacementNode->prev = node->prev;
        replacementNode->next = node->next;
    }
};
