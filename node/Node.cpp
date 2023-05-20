#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <arpa/inet.h>
#include <condition_variable>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ClientWorker.hpp"

class Node {
  public: 
    Node(int id, int port); 
    ~Node(); 

    void StartClient(const std::string& serverPort); 
    void StopClient(); 
    bool IsClientRunning() const; 
 
  private: 
    int id_;
    int port_; 
    std::thread clientThread_; 
    ClientWorker clientWorker_; 
    bool clientRunning_; 

    void ClientThreadFunc(const std::string& serverPort);
};

Node::Node(int id, int port) : id_(id), port_(port), clientWorker_(), clientRunning_(false) {
}

Node::~Node() {
  StopClient(); 
}

void Node::StartClient(const std::string& serverPort) {
  if(!clientRunning_) {
    clientThread_ = std::thread([this, serverPort]() {
      ClientThreadFunc(serverPort); 
    }); 
    clientRunning_ = true; 
  }
}

void Node::StopClient() {
  if (clientRunning_) {
    clientWorker_.Stop(); 
    if(clientThread_.joinable()) {
      clientThread_.join(); 
    }
    clientRunning_ = false; 
  }
}

bool Node::IsClientRunning() const {
  return clientRunning_;
}

void Node::ClientThreadFunc(const std::string& serverPort) {
  clientWorker_.Run(serverPort); 
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: ./main <id> <port>" << std::endl;
    return 1;
  }

  int id = std::atoi(argv[1]); 
  int port = std::atoi(argv[2]);

  Node node(id, port); 

  while(true) {
    if(node.IsClientRunning()) {
      std::cout << "Client running" << std::endl;
      std::cout << "1. Disconnect Client" << std::endl;
    }
    else {
      std::cout << "1. Connect client" << std::endl;
    }
    std::cout << "2. Exit" << std::endl; 
    std::cout << "Choose an option: ";
    int choice;
    std::cin >> choice;

    if (choice == 1 && !node.IsClientRunning()) {
      std::string port;
      std::cout << "Enter server port: ";
      std::cin >> port;
      node.StartClient(port); 
    } 
    else if (choice == 1 && node.IsClientRunning()) {
      node.StopClient(); 
    } 
    else if (choice == 2) {
      node.StopClient(); 
      break;
    } 
    else {
      std::cout << "Invalid choice. Please try again." << std::endl;
    }
  }
  return 0; 
}