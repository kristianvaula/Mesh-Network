#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <sstream>
#include <netdb.h>
#include <stdexcept>
#include <condition_variable>
#include <mutex>

class Node {
  private: 
    int nodeId; 
    int serverSocket; 
    int clientSocket; 
    sockaddr_in nodeAddress{}; 
    std::thread serverThread; 
    std::thread clientThread; 

    std::mutex server_mutex; 
    std::condition_variable cv; 
    std::string serverAddress; 
    std::string serverPort; 

    bool stopServer; 
    bool stopClient; 

  public: 
    Node(int id, int port); 
    void start(); 
    void stop(); 
    void setServerInfo(const std::string& serverAddress, const std::string& serverPort); 

  private: 
    void runServer(); 
    void runClient(); 
    int serverSetup(); 
    int connectToParent(const std::string& serverAddress, const std::string& serverPort); 
    int greetParent(); 
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
  stopServer = false; 
  stopClient = false; 
};

void Node::start() {
  serverThread = std::thread(&Node::runServer, this); 
  clientThread = std::thread(&Node::runClient, this); 
};

void Node::stop() {
  stopServer = true; 
  stopClient = true; 
  serverThread.join(); 
  clientThread.join(); 
  close(serverSocket); 
  close(clientSocket); 
}; 

void Node::setServerInfo(const std::string& serverAddress, const std::string& serverPort) {
  std::unique_lock<std::mutex> lock(server_mutex);
  this->serverAddress = serverAddress; 
  this->serverPort = serverPort; 
  cv.notify_one(); 
}

/* 
Function that the server thread of the node will run. 
Has to handle all operations done by child. 
*/
void Node::runServer() { 
  if(this->serverSetup() == -1){
    return; 
  } 

  while (!stopServer) {
    sockaddr_in clientAddress{}; 
    socklen_t clientAddressSize = sizeof(clientAddress); 
    int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressSize); 
    if (clientSocket < 0) {
      std::cerr << "Failed to accept connection." << std::endl;
      continue;
    }

    std::cout << "Client connected." << std::endl;

    char buffer[1024]; 
    int bytesRead; 

    while ((bytesRead = read(clientSocket, buffer, sizeof(buffer)-1)) > 0 ) {
      buffer[bytesRead] = '\0'; 
      std::cout << "Received data from client: " << buffer << std::endl; 

       // Handle the received data and send a response

      std::string response = "Response from server";
      if (write(clientSocket, response.c_str(), response.length()) < 0) {
          std::cerr << "Failed to send response." << std::endl;
          break;
      }

      memset(buffer, 0, sizeof(buffer));
    }

    if (bytesRead < 0) {
      std::cerr << "Failed to receive data." << std::endl;
    }

    close(clientSocket);
  }
 
}

/* 
Function that the client thread will run in. 
Has to handle all requests by parent server.
*/
void Node::runClient() {
  std::unique_lock<std::mutex> lock(server_mutex); 
  cv.wait(lock, [this]() { return !serverAddress.empty() && !serverPort.empty(); }); 

  if(connectToParent(serverAddress, serverPort) == -1) {
    return; 
  }

  if(this->greetParent() == -1){
    return; 
  } 

  close(clientSocket); 
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
  std::string message = "Hello from Node #";
  std::ostringstream stream; 
  stream << message << this->nodeId; 
  message = stream.str(); 

  if(send(clientSocket, message.c_str(), message.length(), 0) < 0) {
    std::cerr << "Failed to send data" << std::endl; 
    return -1; 
  }

  std::cout << "Message sent to server: " << message << std::endl; 
  return 0; 
}

int Node::connectToParent(const std::string& serverAddress, const std::string& serverPort){
  if(clientSocket != -1) {
    close(clientSocket); 
  }

  addrinfo hints{}; 
  hints.ai_family = AF_INET; 
  hints.ai_socktype = SOCK_STREAM; 

  addrinfo* serverInfo; 
  if (getaddrinfo(serverAddress.c_str(), serverPort.c_str(), &hints, &serverInfo) != 0) {
    std::cerr << "Failed to get server address info" << std::endl;
    return -1;
  }

  clientSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol); 
  if(clientSocket == -1){
    std::cerr << "Failed to create client socket." << std::endl; 
    freeaddrinfo(serverInfo); 
    return -1; 
  }

  int result = connect(clientSocket, serverInfo->ai_addr, serverInfo->ai_addrlen); 
  freeaddrinfo(serverInfo); 

  if(result != 0) {
    std::cerr << "Failed to connect to server" << std::endl; 
    close(clientSocket); 
    clientSocket = -1; 
    return -1; 
  }

  std::cout << "Connected to server" << std::endl; 
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