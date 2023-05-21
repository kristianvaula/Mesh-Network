#include "Node.hpp"
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

    void removeNode(Node* node) {
        if (node == nullptr || head == nullptr || tail == nullptr) return;

        if (node->prev != nullptr) {
            node->prev->next = node->next;
        }
        if (node->next != nullptr) {
            node->next->prev = node->prev;
        }

        if (node == head) {
            head = node->next;
        }
        if (node == tail) {
            tail = node->prev;
        }

        node->prev = nullptr;
        node->next = nullptr;
    }

    //new method
    void replaceNode(Node* node, Node* replacementNode) {
        if (node == nullptr || replacementNode == nullptr) return;
        if (head == nullptr || tail == nullptr) return;
        if(node == replacementNode) return;

        removeNode(replacementNode);

        if (node->prev != nullptr) {
            node->prev->next = replacementNode;
        }
        if (node->next != nullptr) {
            node->next->prev = replacementNode;
        }

        replacementNode->prev = node->prev;
        replacementNode->next = node->next;

        if (node == head) {
            head = replacementNode;
        }
        if (node == tail) {
            tail = replacementNode;
        }

        node->prev = nullptr;
        node->next = nullptr;
    }
};
