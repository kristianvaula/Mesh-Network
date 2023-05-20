#pragma once

#include "Worker.hpp"
#include <functional>

typedef std::uint16_t porttype; 

class ServerWorker : public Worker {
  public:   
    ServerWorker(); 
    ~ServerWorker();

    void RunServer(const std::string& serverPort); 

  private: 
    struct sockaddr_in serverAddress_;

    void HandleClient(int clientSocket); 
    int SetupServer(); 
};

ServerWorker::ServerWorker() : Worker(){}

ServerWorker::~ServerWorker() {}

void ServerWorker::RunServer(const std::string& serverPort) {
  running_.store(true); 

  SetServerport(serverPort);  
  if (SetupServer() != 0) {
    return; 
  }

  while (running_.load()) {
    struct sockaddr_in clientAddress{}; 
    socklen_t clientAddressLength = sizeof(clientAddress); 
    int clientSocket = accept(socket_, (struct sockaddr*)&clientAddress, &clientAddressLength);
    if (clientSocket < 0) {
      std::cerr << "[ServerWorker] Failed to accept client connection" << std::endl;
      continue;
    }
    std::cout << "[ServerWorker] Accepted client connection" << std::endl;
    std::thread clientThread(&ServerWorker::HandleClient,this, clientSocket);
    clientThread.detach();
  }

  close(socket_); 
  return; 
}

void ServerWorker::HandleClient(int clientSocket) {
  char buffer[1024] = { 0 }; 
  int bytesRead; 
  while ((bytesRead = read(clientSocket, buffer, 1024)) > 0) {
    std::cout << "[ServerWorker] Received message from client: " << buffer << std::endl;
    std::string response = "Hello from server";
    int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
    if (bytesSent <= 0) {
      std::cerr << "[ServerWorker] Failed to send response" << std::endl;
    }
    memset(buffer, 0, sizeof(buffer));
  }
  close(clientSocket);
}

int ServerWorker::SetupServer() {
  porttype serverPort; 
  {
    std::unique_lock<std::mutex> lock(mutex_); 
    serverPort = serverPort_; 
  }

  socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_ == -1) {
      std::cerr << "[ServerWorker] Failed to create server socket." << std::endl;
      return 1;
  }

  {
    std::unique_lock<std::mutex> lock(mutex_); 
    std::memset(&serverAddress_, 0, sizeof(serverAddress_));
    serverAddress_.sin_family = AF_INET;
    serverAddress_.sin_addr.s_addr = INADDR_ANY;
    serverAddress_.sin_port = htons(serverPort);
  }
  
  if (bind(socket_, (struct sockaddr*)&serverAddress_, sizeof(serverAddress_)) < 0){
    std::cerr << "[ServerWorker] Failed to bind server socket" << std::endl;
    close(socket_);
    return 1;
  }

  if (listen(socket_, 5) < 0) {
    std::cerr << "[ServerWorker] Failed to listen on server socket" << std::endl;
    close(socket_);
    return 1;
  }

  std::cout << "[ServerWorker] Listening on port " << serverPort << std::endl;
  return 0; 
}
