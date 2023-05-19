#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <chrono>
#include <list>
#include <sstream>
#include <netdb.h>
#include <string_view>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <../model/dto/SocketData.h>

class Node {
  private: 
    int nodeId; 
    int x; 
    int serverSocket; 
    int childSocket; 
    int clientSocket; 
    sockaddr_in nodeAddress{}; 
    std::string serverAddress; 
    std::string serverPort; 

    std::thread serverThread; 
    std::thread clientThread; 

    std::mutex mutex; 
    std::condition_variable cv; 
    std::atomic<bool> running; 
    std::list<NodeData> serverJobs; 

  public: 
    Node(int id, int port); 
    void start(); 
    void stop(); 
    void setServerInfo(const std::string& serverAddress, const std::string& serverPort); 

  private: 
    void runServer(); 
    void runClient(); 
    void simulateMovement(int x);
    void serializeStruct(const NodeData& data, char* buffer) ; 
    int handleAction(const NodeData& socketData);
    int serverSetup(); 
    int connectToParent(const std::string& serverAddress, const std::string& serverPort); 
    int greetParent(); 
    int returnOK();
    int pushServerjob(const NodeData& socketData); 
};

Node::Node(int id, int port ) {
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
      std::cerr << "Failed to create server socket." << std::endl;
      return;
  }

  nodeAddress.sin_family = AF_INET; 
  nodeAddress.sin_addr.s_addr = INADDR_ANY; 
  nodeAddress.sin_port = htons(port); 

  nodeId = id; 
  x = 0; 
  running = true; 

  if(this->serverSetup() == -1){
    return; 
  } 
};

void Node::start() {
  serverThread = std::thread(&Node::runServer, this); 
  clientThread = std::thread(&Node::runClient, this); 
};

void Node::stop() {
  {
    std::unique_lock<std::mutex> lock(mutex); 
    running = false; 
  }
  serverThread.join(); 
  clientThread.join(); 
  close(serverSocket); 
  close(clientSocket); 
}; 

void Node::setServerInfo(const std::string& serverAddress, const std::string& serverPort) {
  {
    std::unique_lock<std::mutex> lock(mutex);
    this->serverAddress = serverAddress; 
    this->serverPort = serverPort; 
  }
  cv.notify_one(); 
}

/* 
Function that the server thread of the node will run. 
Has to handle all operations done by child. 
*/
void Node::runServer() { 

  while (running) {
    sockaddr_in clientAddress{}; 
    socklen_t clientAddressSize = sizeof(clientAddress); 
    int childSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressSize); 
    if (childSocket < 0) {
      std::cerr << "Failed to accept connection." << std::endl;
      continue;
    }

    std::cout << "Client connected." << std::endl;

    NodeData socketData; 
    int bytesRead; 

    while ((bytesRead = read(clientSocket, &socketData, sizeof(socketData))) > 0) {
      // Handle the received data and send a response
      std::unique_lock<std::mutex> lock(mutex);
      if(!serverJobs.empty()){
        const NodeData& instruction = serverJobs.front();
        char serializedData[sizeof(NodeData)]; 
        serializeStruct(instruction, serializedData); 
        write(childSocket, serializedData, sizeof(serializedData)); 
        serverJobs.pop_front();
      }
    
    }

    if (bytesRead < 0) {
      std::cerr << "Failed to receive data." << std::endl;
    }

    close(childSocket);
  }
 
}

/* 
Function that the client thread will run in. 
Has to handle all requests by parent server.
*/
void Node::runClient() {
  std::unique_lock<std::mutex> lock(mutex); 
  cv.wait(lock, [this]() { return !serverAddress.empty() && !serverPort.empty(); }); 

  if(connectToParent(serverAddress, serverPort) == -1) {
    return; 
  }

  if(this->greetParent() == -1){
  return; 
  } 

  while(running) {
    NodeData socketData; 
    int bytesRead = read(clientSocket, &socketData, sizeof(socketData)); 
    
    if (bytesRead < 0) {
      std::cerr << "Failed to receive data." << std::endl;
      break;
    } else if (bytesRead == 0) {

      std::cout << "Connection closed by the server." << std::endl;
      break;
    }

    handleAction(socketData); 
  }
}

int Node::handleAction(const NodeData& socketData) {
  if(socketData.nodeId != nodeId){
      std::cout << "Received passthrough" << std::endl;
      pushServerjob(socketData);  
  }

  std::cout << "Received request with action: " << socketData.action[0] << std::endl;

  std::string divider = "MOVETO_"; 
  size_t pos = socketData.action.find(divider);  

  if (pos != std::string::npos) { // Case MOVETO
    int coordinate = std::stoi(socketData.action.substr(divider.length(), socketData.action.length() - 1));
    simulateMovement(coordinate);
    std::string portnr = std::to_string(socketData.port);
    if(connectToParent(socketData.ipAddress, portnr) != -1) {
      returnOK(); 
      return 1; 
    }
    else return -1; 
  } else {
    if (socketData.action == "HELLO") {
      // Handle 'HELLO' action
    } else if (socketData.action == "REMOVE_NODE") {
      // Handle 'REMOVE_NODE' action
    } else {
      std::cout << "Received unknown action from server: " << socketData.action << std::endl;
      return 1; 
    }
  } 
}

/*
Simulates the time it takes for node to move  
*/
void Node::simulateMovement(int position) {
  x = position; 
  std::cout << "Movement to x:" << position << std::endl; 
  for (size_t i = 0; i < 5; i++){
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "."; 
  }
  std::cout << std::endl; 
}

void Node::serializeStruct(const NodeData& data, char* buffer) {
  std::memcpy(buffer, &data, sizeof(NodeData));
}

int Node::serverSetup(){
  if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&nodeAddress), sizeof(nodeAddress)) < 0){
    std::cerr << "Failed to bind socket." << std::endl;
    return -1;
  }

  if(listen(serverSocket, 1) < 0) {
    std::cerr << "Failed to listen." << std::endl;
    return -1;
  }

  std::cout << "Node listening on IP: " << inet_ntoa(nodeAddress.sin_addr) << " and port: " << ntohs(nodeAddress.sin_port) << std::endl; 
  return 0; 
}

int Node::greetParent(){
  NodeData helloMessage;  
  helloMessage.nodeId = nodeId; 
  helloMessage.action = "HELLO"; 
  std::ostringstream stream; 

  char serializedData[sizeof(NodeData)]; 
  serializeStruct(helloMessage, serializedData); 
  if(write(clientSocket, serializedData, sizeof(serializedData)) < 0) {
    std::cerr << "Failed to send data" << std::endl; 
    return -1; 
  }

  std::cout << "Hello sent to server" << std::endl; 
  return 0; 
}

int Node::returnOK() {
  NodeData helloMessage;  
  helloMessage.nodeId = nodeId; 
  helloMessage.action = "OK"; 
  std::ostringstream stream; 

  char serializedData[sizeof(NodeData)]; 
  serializeStruct(helloMessage, serializedData); 
  if(write(clientSocket, serializedData, sizeof(serializedData)) < 0) {
    std::cerr << "Failed to send data" << std::endl; 
    return -1; 
  }

  std::cout << "Hello sent to server" << std::endl; 
  return 0; 
}

int Node::connectToParent(const std::string& serverAddress, const std::string& serverPort){
  addrinfo hints{}; 
  hints.ai_family = AF_INET; 
  hints.ai_socktype = SOCK_STREAM; 

  addrinfo* serverInfo; 
  if (getaddrinfo(serverAddress.c_str(), serverPort.c_str(), &hints, &serverInfo) != 0) {
    std::cerr << "Failed to get server address info" << std::endl;
    return -1;
  }

  int newClientSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol); 
  if(newClientSocket == -1){
    std::cerr << "Failed to create client socket." << std::endl; 
    freeaddrinfo(serverInfo); 
    return -1; 
  }

  int result = connect(newClientSocket, serverInfo->ai_addr, serverInfo->ai_addrlen); 
  freeaddrinfo(serverInfo); 

  if(result != 0) {
    std::cerr << "Failed to connect to server" << std::endl; 
    close(newClientSocket); 
    return -1; 
  }

  std::cout << "Connected to server" << std::endl; 
  
  if(clientSocket != -1) {
    close(clientSocket); 
  }

  clientSocket = newClientSocket; 

  return 0; 
}

int Node::pushServerjob(const NodeData& socketData) {
  std::unique_lock<std::mutex> lock(mutex);
  serverJobs.push_front(socketData); 
  cv.notify_one(); 
  return 0; 
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: ./main <id> <port>" << std::endl;
    return 1;
  }

  int id = std::atoi(argv[1]); 
  int port = std::atoi(argv[2]); 
  
  Node node(id,port); 
  node.start();

  while(true) {
    std::cout << "1. Connect to server" << std::endl;
    std::cout << "2. Exit" << std::endl;
    std::cout << "Choose an option: ";
    int choice;
    std::cin >> choice;

    if (choice == 1) {
      std::string address, port;
      std::cout << "Enter server address: ";
      std::cin >> address;
      std::cout << "Enter server port: ";
      std::cin >> port;
      node.setServerInfo(address, port);
      break;
    } 
    else if (choice == 2) {
      node.stop();
      break;
    } 
    else {
      std::cout << "Invalid choice. Please try again." << std::endl;
    }
  }
  return 0; 
}