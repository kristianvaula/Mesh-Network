#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

class Node {
  private: 
    int nodeId; 
    int serverSocket; 
    int clientSocket; 
    sockaddr_in nodeAddress{}; 
    sockaddr_in serverAddress{}; 
    std::thread serverThread; 
    std::thread clientThread; 
    bool stopServer; 
    bool stopClient; 

  public: 
    Node(int id, int port); 
    void start(); 
    void stop(); 

  private: 
    void runServer(); 
    void runClient(); 
    int serverSetup(); 
    void greetParent(); 
};

Node::Node(int id, int port ) {
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
      std::cerr << "Failed to create server socket." << std::endl;
      return;
  }

  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == -1) {
      std::cerr << "Failed to create client socket." << std::endl;
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

/* 
Function that the server thread of the node will run. 
Has to handle all operations done by child. 
*/
void Node::runServer() { 
  if(this->serverSetup() == -1){
    return; 
  } 
  std::cout << "Server listening for connections." << std::endl;

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
    memset(buffer, 0, sizeof(buffer)); 
    int bytesRead; 

    while ((bytesRead = read(clientSocket, buffer, sizeof(buffer)-1)) > 0 ) {
      std::cout << "Received data from client: " << buffer << std::endl; 

       // Handle the received data and send a response

      const char* response = "Response from server";
      if (send(clientSocket, response, strlen(response), 0) < 0) {
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
  if(connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) {
    std::cerr << "Failed to connect to server" << std::endl; 
    return; 
  }

  std::cout << "Connected to server" << std::endl; 

  const char* message = "Hello from node"; 

  if(send(clientSocket, message, strlen(message), 0) < 0) {
    std::cerr << "Failed to send data" << std::endl; 
    return; 
  }

  std::cout << "Message sent to server: " << message << std::endl; 

  char buffer[1024]; 

  int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0); 
  if(bytesRead < 0) {
    std::cerr << "Failed to receive data from the server" << std::endl; 
    return; 
  }

  buffer[bytesRead] = '\0'; 

  std::cout << "Response from server" << std::endl; 

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
  node.stop(); 
  return 0; 
}