#pragma once
#include "Worker.hpp"
#include <functional>
#include <set> 

typedef std::uint16_t porttype; 

class ServerWorker : public Worker {
  public:   
    ServerWorker(std::queue<std::string>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv); 
    ~ServerWorker();

    void RunServer(const std::string& serverPort); 

  private: 
    struct sockaddr_in serverAddress_;
    std::mutex clientSocketsMutex_; 
    std::set<int> connectedClientSockets_; 

    void BroadcastMessages(); 
    void HandleClient(int clientSocket); 
    int SetupServer(); 
};

ServerWorker::ServerWorker(std::queue<std::string>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv) 
: Worker(messageQueue, messageMutex, cv), clientSocketsMutex_(), connectedClientSockets_() {
}

ServerWorker::~ServerWorker() {}

void ServerWorker::RunServer(const std::string& serverPort) {
  running_.store(true); 

  SetServerport(serverPort);  
  if (SetupServer() != 0) {
    return; 
  }

  std::thread broadcastThread(&ServerWorker::BroadcastMessages, this); 
  broadcastThread.detach(); 

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

void ServerWorker::BroadcastMessages() {
  while (running_.load()) {
    std::queue<std::string> messages; 
    {
      std::unique_lock<std::mutex> lock(*messageMutex_);
      while(messageQueue_.empty() && running_.load()){
        cv_->wait(lock, [this] { return !messageQueue_.empty() || !running_.load(); });
      }
      while (!messageQueue_.empty()) {
        messages.push(messageQueue_.front());
        messageQueue_.pop();
      }
    }

    while (!messages.empty()) {
      std::string message = std::move(messages.front()); 
      messages.pop(); 

      {
        std::unique_lock<std::mutex> lock(clientSocketsMutex_); 
        for(const auto& clientSocket : connectedClientSockets_) {
          int bytesSent = send(clientSocket, message.c_str(), message.length(), 0); 
          if (bytesSent <= 0) {
            std::cerr << "[Broadcast] Failed to send message" << std::endl;
            break; 
          }
        }
      }
    }
  }
}

void ServerWorker::HandleClient(int clientSocket) {
  {
    std::unique_lock<std::mutex> lock(clientSocketsMutex_); 
    connectedClientSockets_.insert(clientSocket); 
  }
  char buffer[1024] = { 0 }; 
  int bytesRead; 
  while ((bytesRead = read(clientSocket, buffer, 1024)) > 0) {
    std::cout << "[Client] " << buffer << std::endl;
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
