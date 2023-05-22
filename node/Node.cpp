//
// Created by Kristian Vaula Jensen, May 2023
//
#include "Node.hpp"

Node::Node(int id, porttype port, std::atomic<int>* instructionFlag,  std::queue<NodeData>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv) 
: id_(id), 
  port_(port), 
  xPosition_(0), 
  clientRunning_(false), 
  serverRunning_(false),
  serverWorker_(&id_, &port_, &serverRunning_, instructionFlag, messageQueue,messageMutex,cv),
  clientWorker_(&id_, &port_, &clientRunning_, &xPosition_, instructionFlag, messageQueue,messageMutex,cv)
  {
    StartServer(); 
  }

Node::~Node() {
  Stop(); 
}

void Node::Stop() {
  StopServer(); 
  StopClient(); 
}

void Node::StartClient(const std::string& serverPort) {
  if(!clientRunning_.load()) {
    clientThread_ = std::thread(&Node::ClientThreadMethod, this, serverPort); 
  }
}

void Node::StartServer() {
  const std::string port = std::to_string(port_.load()); 
  if(!serverRunning_.load()) {
    serverThread_ = std::thread(&Node::ServerThreadMethod, this, port);
  }
}

void Node::StopClient() {
  if (clientRunning_) {
    clientWorker_.Stop();  
    if(clientThread_.joinable()) {
      clientThread_.join(); 
    }
    std::cout << "[ Node ] Client joined." << std::endl;
  }
}

void Node::StopServer() {
  if (serverRunning_) {
    serverWorker_.Stop();  
    if(serverThread_.joinable()) {
      serverThread_.join(); 
    }
    std::cout << "[ Node ] Server joined." << std::endl;
  }
}

void Node::ServerThreadMethod(const std::string &serverPort) {
  serverWorker_.RunServer(serverPort); 
}

void Node::ClientThreadMethod(const std::string &serverPort) {
  clientWorker_.RunClient(serverPort); 
}

bool Node::IsClientRunning() const {
  return clientRunning_.load();
}

bool Node::IsServerRunning() const {
  return serverRunning_.load();
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: ./main <id> <port>" << std::endl;
    return 1;
  }

  int id = std::atoi(argv[1]); 
  porttype port = static_cast<porttype>(std::atoi(argv[2]));
  std::queue<NodeData> messageQueue; 
  std::mutex messageMutex; 
  std::condition_variable cv; 
  std::atomic<int> instructionSucceeded(-2); 

  Node node(id, port, &instructionSucceeded, messageQueue, &messageMutex,&cv); 

  //Delay to ensure server print status before menu
  std::this_thread::sleep_for(std::chrono::milliseconds(400)); 
  while(true) {
    std::cout << "[ Node ] 'c' - Connect client" << std::endl;
    std::cout << "[ Node ] 'q' - Exit" << std::endl; 
    std::cout << "[ Node ] Choose an option: " << std::endl;
    char choice;
    std::cin >> choice;
    
    if (choice == 'c' && !node.IsClientRunning()) {
      std::string port;
      std::cout << "[ Node ] Enter server port the client should connect to: ";
      std::cin >> port;
      node.StartClient(port); 
    } 
    else if (choice == 'q') {
      node.Stop();  
      break; 
    } 
    else {
      std::cout << "[ Node ] Invalid choice. Please try again." << std::endl;
    }
  }
  std::cout << "[ Node ] Shutting down" << std::endl;
  return 0; 
}