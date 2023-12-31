//
// Created by Kristian Vaula Jensen, May 2023
//
#include "ServerWorker.hpp"

ServerWorker::ServerWorker(std::atomic<int>* nodeId, std::atomic<porttype>* port, std::atomic<bool>* running, std::atomic<int>* instructionFlag, std::queue<NodeData>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv) 
: Worker(nodeId, port, running, instructionFlag, messageQueue, messageMutex, cv), clientSocketsMutex_(), connectedClientSockets_() {
}

ServerWorker::~ServerWorker() {}

void ServerWorker::RunServer(const std::string& serverPort) {
  running_->store(true); 
  std::vector<std::thread> handlerThreads; 

  SetServerport(serverPort);  
  if (SetupServer() != 0) {
    return; 
  }

  std::thread broadcastThread(&ServerWorker::HandleInstructions, this); 

  //Set timeout
  timeval timeout; 
  timeout.tv_sec = 3; 
  timeout.tv_usec = 0; 

  while (running_->load()) {
    //Confgure timeout
    timeval timeoutCopy = timeout; 

    fd_set readfds; 
    FD_ZERO(&readfds); 
    FD_SET(socket_, &readfds); 

    int selectResult = select(socket_ + 1, &readfds, nullptr, nullptr, &timeoutCopy); 
    if (selectResult == -1) {
      std::cerr << "[Error] Error in select" << std::endl; 
      break; 
    }
    else if (selectResult == 0) {
      continue; 
    }

    if (FD_ISSET(socket_, &readfds)) {
      struct sockaddr_in clientAddress{}; 
      socklen_t clientAddressLength = sizeof(clientAddress); 
      int clientSocket = accept(socket_, (struct sockaddr*)&clientAddress, &clientAddressLength);
      if (clientSocket < 0) {
        std::cerr << "[Error] Failed to accept client connection" << std::endl;
        continue;
      }
      else {
        std::cout << "[ Node ] Accepted client connection" << std::endl;
        std::thread handlerThread(&ServerWorker::HandleClient,this, clientSocket);
        handlerThreads.push_back(std::move(handlerThread)); 
      }
    }
  }

  for (auto& thread : handlerThreads) {
    if (thread.joinable()) {
      thread.join(); 
    }
  }

  if (broadcastThread.joinable()) {
    broadcastThread.join(); 
  }

  close(socket_); 
  std::cout << "[ Node ] Server closed" << std::endl;
  return; 
}

void ServerWorker::HandleInstructions() {
  while (running_->load()) {
    std::queue<NodeData> messages; 
    {
      std::unique_lock<std::mutex> lock(*messageMutex_);
      while(messageQueue_.empty() && running_->load()){
        cv_->wait(lock, [this] { return !messageQueue_.empty() || !running_->load(); });
      }
      while (!messageQueue_.empty()) {
        messages.push(messageQueue_.front());
        messageQueue_.pop();
      }
    }

    while (!messages.empty()) {
      NodeData message = std::move(messages.front()); 
      messages.pop(); 

      //Replace self instruction 
      if (message.nodeId == nodeId_->load() && actionFromString(message.action) == ActionType::REPLACE_SELF){ // Check if message is intended for itself
        int destination = 0; 
        std::string action = message.action; 
        size_t underscorePos = action.rfind('_'); 
        if (underscorePos != std::string::npos) {
          std::string arg = action.substr(underscorePos+1); 
          try{
            destination = std::stoi(arg); 
          }
          catch(std::invalid_argument e){
            std::cerr << "[Error] Received invalid positon" << std::endl; 
          }
          catch(std::out_of_range e){
            std::cerr << "[Error] Received invalid positon" << std::endl;
          }
        }

        int response = HandleReplaceSelf(message.port, destination);
        instructionFlag_->store(response); 
        cv_->notify_all(); 
        
      }
      else { 
        std::unique_lock<std::mutex> lock(clientSocketsMutex_); 
        for(const auto& clientSocket : connectedClientSockets_) {
          int bytesSent = send(clientSocket, &message, sizeof(message), 0);
          if (bytesSent <= 0) {
            std::cerr << "[Error] Failed to send message" << std::endl;
            break; 
          }
        }
      }
    }
  }
}

void ServerWorker::HandleClient(int clientSocket) {
  NodeData nodeData = { 0 }; 
  fd_set readfds; 
  FD_ZERO(&readfds); 
  FD_SET(clientSocket, &readfds); 

  timeval timeout; 
  timeout.tv_sec = 3; 
  timeout.tv_usec = 0; 

  int selectResult = select(clientSocket+1, &readfds, nullptr, nullptr, &timeout); 
  
  if (selectResult == -1) {
    std::cerr << "[Error] Error in select" << std::endl;
    close(clientSocket);
    return;
  }

  if (selectResult == 0) {
    std::cout << "[ Node ] Receive timeout" << std::endl;
    close(clientSocket);
    return;
  }

  int bytesRead = recv(clientSocket, &nodeData, sizeof(nodeData), 0); 
  if (bytesRead > 0) {
    if(actionFromString(nodeData.action) == ActionType::HELLO) {
      {
        std::unique_lock<std::mutex> lock(clientSocketsMutex_); 
        connectedClientSockets_.insert(clientSocket); 
      }
      return; 
    }
  }
  //Else close 
  close(clientSocket); 
}

int ServerWorker::HandleReplaceSelf(int port, int pos) {
  int bytesSent, replaceMentId, replacementSocket, replacementPort, bytesReceived = 0; 
  std::stringstream ss; 
  NodeData nodeData = { 0 }; 
  nodeData.nodeId = -1; //All nodes will process this
  ss << actionToString(ActionType::MOVETO) << "_" << pos;  
  std::strcpy(nodeData.action, ss.str().c_str()); 
  nodeData.port = port; 
  
  std::unique_lock<std::mutex> lock(clientSocketsMutex_); 

  //Check if need replacement 
  if (connectedClientSockets_.size() <= 0) {
    instructionFlag_->store(0); 
    return 0; 
  }

  //Find replacement 
  std::cout << "[ Node ] Finding Replacement" << std::endl; 
  for(const auto& clientSocket : connectedClientSockets_) {
    //Send request
    if ((bytesSent = send(clientSocket, &nodeData, sizeof(nodeData), 0)) <= 0) {
      std::cerr << "[Error] Send failed, removing client" << std::endl; 
      connectedClientSockets_.erase(clientSocket); 
      close(clientSocket); 
      continue; 
    }
    nodeData = { 0 }; 
    //Receive reponse 
    fd_set readfds; 
    FD_ZERO(&readfds); 
    FD_SET(clientSocket, &readfds); 

    timeval timeout; 
    timeout.tv_sec = 5; 
    timeout.tv_usec = 0; 

    int selectResult = select(clientSocket+1, &readfds, nullptr, nullptr, &timeout); 
    
    if (selectResult == -1) {
      std::cerr << "[Error] Error in select" << std::endl;
      close(clientSocket);
      return -1;
    }

    if (selectResult == 0) {
      std::cout << "[ Node ] Receive timeout" << std::endl;
      close(clientSocket);
      return -1;
    }


    if ((bytesReceived = recv(clientSocket, &nodeData, sizeof(nodeData), 0)) <= 0) {
      std::cerr << "[Error] Response failed, removing client" << std::endl;
      close(clientSocket); 
      connectedClientSockets_.erase(clientSocket); 
      continue;  
    }

    if(actionFromString(nodeData.action) == ActionType::OK) {
      replaceMentId = nodeData.nodeId; 
      replacementSocket = clientSocket; 
      replacementPort = nodeData.port; 
      break; 
    }
  }

  nodeData = { 0 }; 
  nodeData.nodeId = -1; //All nodes will process this
  ss.str("");
  ss << actionToString(ActionType::MOVETO) << "_" << -1; //Nodes will not change position
  std::strcpy(nodeData.action, ss.str().c_str()); 
  nodeData.port = replacementPort; 

  //Rest of children nodes should change connection to replacement node 
  for(const auto& clientSocket : connectedClientSockets_) {
    if (clientSocket == replacementSocket) {
      continue; 
    }

    if ((bytesSent = send(clientSocket, &nodeData, sizeof(nodeData), 0)) <= 0) {
      std::cerr << "[Error] Send failed, removing client" << std::endl; 
      close(clientSocket); 
      connectedClientSockets_.erase(clientSocket); 
      continue; 
    }
  }

  return replaceMentId; 
}

int ServerWorker::SetupServer() {
  porttype serverPort; 
  {
    std::unique_lock<std::mutex> lock(workerMutex_); 
    serverPort = serverPort_; 
  }

  socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_ == -1) {
      std::cerr << "[Error] Failed to create server socket." << std::endl;
      return 1;
  }

  {
    std::unique_lock<std::mutex> lock(workerMutex_); 
    std::memset(&serverAddress_, 0, sizeof(serverAddress_));
    serverAddress_.sin_family = AF_INET;
    serverAddress_.sin_addr.s_addr = INADDR_ANY;
    serverAddress_.sin_port = htons(serverPort);
  }
  
  timeval timeout; 
  timeout.tv_sec = 3; 

  setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout)); 

  if (bind(socket_, (struct sockaddr*)&serverAddress_, sizeof(serverAddress_)) < 0){
    std::cerr << "[Error] Failed to bind server socket" << std::endl;
    close(socket_);
    return 1;
  }

  if (listen(socket_, 5) < 0) {
    std::cerr << "[Error] Failed to listen on server socket" << std::endl;
    close(socket_);
    return 1;
  }

  std::cout << "[ Node ]Listening on port " << serverPort << std::endl;
  return 0; 
}