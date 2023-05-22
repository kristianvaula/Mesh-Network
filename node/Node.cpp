//
// Created by Kristian Vaula Jensen, May 2023
//
#include "ClientWorker.hpp"
#include "ServerWorker.hpp"


typedef std::uint16_t porttype; 

class Node {
  public: 
    Node(int id, int port, std::queue<NodeData>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv); 
    ~Node(); 

    void Stop(); 
    void StartClient(const std::string& serverPort); 
    void StartServer();
    void StopClient(); 
    void StopServer(); 
    bool IsClientRunning() const; 
    bool IsServerRunning() const; 
 
  private: 
    int id_;
    int port_; 
    std::atomic<int> xPosition_; 
    std::thread serverThread_; 
    std::thread clientThread_; 
    ServerWorker serverWorker_; 
    ClientWorker clientWorker_; 
    bool clientRunning_; 
    bool serverRunning_; 

};

Node::Node(int id, int port, std::queue<NodeData>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv) 
: id_(id), port_(port), xPosition_(0), serverWorker_(&id_, &port_, messageQueue,messageMutex,cv), clientWorker_(&id_, &port_, &xPosition_, messageQueue,messageMutex,cv), clientRunning_(false), serverRunning_(false) {
}

Node::~Node() {
  Stop(); 
}

void Node::Stop() {
  StopServer(); 
  StopClient(); 
}

void Node::StartClient(const std::string& serverPort) {
  if(!clientRunning_) {
    clientThread_ = std::thread(&ClientWorker::RunClient, &clientWorker_, serverPort); 
    clientRunning_ = true; 
  }
}

void Node::StartServer() {
  const std::string port = std::to_string(port_); 
  if(!serverRunning_) {
    serverThread_ = std::thread(&ServerWorker::RunServer, &serverWorker_, port);
    serverRunning_ = true; 
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

void Node::StopServer() {
  if (serverRunning_) {
    serverWorker_.Stop(); 
    if(serverThread_.joinable()) {
      serverThread_.join(); 
    }
    serverRunning_ = false; 
  }
}

bool Node::IsClientRunning() const {
  return clientRunning_;
}

bool Node::IsServerRunning() const {
  return serverRunning_;
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: ./main <id> <port>" << std::endl;
    return 1;
  }

  int id = std::atoi(argv[1]); 
  int port = std::atoi(argv[2]);
  std::queue<NodeData> messageQueue; 
  std::mutex messageMutex; 
  std::condition_variable cv; 


  Node node(id, port, messageQueue, &messageMutex,&cv); 
  node.StartServer(); 

  std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
  while(true) {
    std::cout << "1. Connect client" << std::endl;
    std::cout << "2. Exit" << std::endl; 
    std::cout << "Choose an option: " << std::endl;
    int choice;
    std::cin >> choice;
    
    if (choice == 1 && !node.IsClientRunning()) {
      std::string port;
      std::cout << "Enter server port the client should connect to: ";
      std::cin >> port;
      node.StartClient(port); 
    } 
    else if (choice == 2) {
      node.Stop();  
      break; 
    } 
    else {
      std::cout << "Invalid choice. Please try again." << std::endl;
    }
  }
  return 0; 
}