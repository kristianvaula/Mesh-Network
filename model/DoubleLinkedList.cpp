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
            // List is empty
            head = tail = node;
        } else {
            node->next = head;
            head->prev = node;
            head = node;
        }
    }

    void insertEnd(Node* node) {
        if (tail == nullptr) {
            // List is empty
            head = tail = node;
        } else {
            node->prev = tail;
            tail->next = node;
            tail = node;
        }
    }

    void displayForward() {
        Node* current = head;
        while (current != nullptr) {
            current = current->next;
        }
        std::cout << std::endl;
    }

    void displayBackward() {
        Node* current = tail;
        while (current != nullptr) {
            current = current->prev;
        }
        std::cout << std::endl;
    }
};
