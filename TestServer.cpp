#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include "model/dto/SocketData.h"

typedef std::uint16_t porttype;

void HandleClient(int clientSocket) {
  NodeData nodeData = { 0 }; 
  int bytesRead; 
  int i = 0; 
  while ((bytesRead = recv(clientSocket, &nodeData, sizeof(nodeData), 0)) > 0) {
    std::cout << "[Server] Received message from client: " << nodeData.action << std::endl;
    NodeData nodeDataOut = { 0 }; 
    if(i == 0) {
      std::string actionStr = "HELLO"; 
      strcpy(nodeData.action, actionStr.c_str()); 
      nodeData.nodeId = 1; 
      nodeData.port = 3000; 
      int bytesSent = send(clientSocket, &nodeData, sizeof(nodeData), 0); 
    }
    else if(i == 1){
      break; 
    }
    
    i++; 
    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
  }
  close(clientSocket);
}

int main() {
  porttype serverPort = 12345;

  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
    std::cerr << "[Server] Failed to create server socket" << std::endl;
    return 1;
  }

  struct sockaddr_in serverAddress{};
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(serverPort);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    std::cerr << "[Server] Failed to bind server socket" << std::endl;
    close(serverSocket);
    return 1;
  }

  if (listen(serverSocket, 5) < 0) {
    std::cerr << "[Server] Failed to listen on server socket" << std::endl;
    close(serverSocket);
    return 1;
  }

  std::cout << "[Server] Listening on port " << serverPort << std::endl;

  while (true) {
    struct sockaddr_in clientAddress{};
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
    if (clientSocket < 0) {
      std::cerr << "[Server] Failed to accept client connection" << std::endl;
      continue;
    }
    std::cout << "[Server] Accepted client connection" << std::endl;

    std::thread clientThread(HandleClient, clientSocket);
    clientThread.detach();
  }

  close(serverSocket);
  return 0;
}