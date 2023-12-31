//
// Created by Kristian Vaula Jensen, May 2023
//
#include "ClientWorker.hpp"

ClientWorker::ClientWorker(std::atomic<int>* nodeId, std::atomic<porttype>* port, std::atomic<bool>* running, std::atomic<int>* xPosition, std::atomic<int>* instructionFlag,  std::queue<NodeData>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv) 
: xPosition_(xPosition), Worker(nodeId, port, running, instructionFlag, messageQueue, messageMutex,cv){
}

ClientWorker::~ClientWorker() {}

void ClientWorker::RunClient(const std::string& serverPort) {
  running_->store(true); 

  SetServerport(serverPort);  
  if (Connect() != 0) {
    return; 
  }

  NodeData nodeData = { 0 }; 
  int bytesRead; 
  bool passthrough; 

  //Set timeout
  timeval timeout; 
  timeout.tv_sec = 3; 
  timeout.tv_usec = 0; 

  while (running_->load()) {
    timeval timeoutCopy = timeout; 

    fd_set readfds; 
    FD_ZERO(&readfds); 
    FD_SET(socket_, &readfds); 

    int selectResult = select(socket_+1, &readfds, nullptr, nullptr, &timeout); 
  
    if (selectResult == -1) {
      std::cerr << "[Error] Error in select" << std::endl;
      close(socket_);
      return;
    }

    else if (selectResult == 0) {
      continue;
    }

    //Read nodeData
    bytesRead = recv(socket_, &nodeData, sizeof(nodeData), 0); 
    if(bytesRead > 0) {
      //Check if intended for this node
      passthrough = IsPassthrough(nodeData.nodeId); 

      if (passthrough){ //If not this node
        // Send to serverThread queue
        EnqueueInstruction(nodeData);  
      }
      else { //Else handle
        HandleAction(nodeData); 
      }
    }
    else if (bytesRead == 0) {
      std::cout << "[ Node ] Received empty server message" << std::endl;
      break;  
    }
  }

  close(socket_);  
  std::cout << "[ Node ] Client closed" << std::endl; 
}

void ClientWorker::HandleAction(NodeData& nodeData) {
  ActionType action = actionFromString(nodeData.action); 
  switch (action) {
    case (ActionType::HELLO): // Hello 
      std::cout << "[Server] Hello" << std::endl; 
      break; 
    case (ActionType::MOVETO):// Move to <x> command 
      std::cout << "[Server] Move To Request" << std::endl;
      if (HandleMoveTo(nodeData) != 0){
        SendNone(); 
      } 
      break; 
    case (ActionType::REMOVE_NODE): 
      std::cout << "[Server] Remove Request" << std::endl;
      HandleRemoveNode(nodeData); 
      break; 
    case (ActionType::OK): 
      std::cout << "[Server] OK" << std::endl;
      break; 
    default: 
      std::cout << "[Server] Invalid action type" << std::endl; 
      break; 
  }
}

void ClientWorker::SimulateMovement(int pos) {
  std::cout << "[ Node ] Moving."; 
  for (uint8_t i = 0; i < 3; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "."; 
  }
  std::cout << std::endl; 
  if(pos != -1) {
    xPosition_->store(pos); 
  }
  std::cout << "[ Node ] Moved To " << pos << std::endl; 
}

bool ClientWorker::IsPassthrough(int nodeId) {
  bool result = false; 
  if (nodeId != nodeId_->load() && nodeId != -1) {
    result = true; 
  } 
  return result; 
}

int ClientWorker::Connect() {
  porttype serverPort; 
  {
    std::unique_lock<std::mutex> lock(workerMutex_); 
    serverPort = serverPort_; 
  }

  if (socket_ != -1) {
    close(socket_);
    socket_ = -1;
  }

  socket_ = socket(AF_INET, SOCK_STREAM, 0); 
  if (socket_ == -1) {
    std::cout << "[ Node ] Failed to connect to parent, returning" << std::endl;
    return 1;
  }

  //Consider adding options 

  struct sockaddr_in serverAddress; 
  serverAddress.sin_family = AF_INET; 
  serverAddress.sin_port = htons(serverPort); 
  if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0) {
    std::cerr << "[Error] Failed to convert address" << std::endl; 
    close(socket_); 
    return 1;  
  }

  if (connect(socket_, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    std::cerr << "[Error] Failed to connect to the server" << std::endl; 
    close(socket_); 
    return 1; 
  }

  std::cout << "[ Node ] Connected to server: " << serverPort << std::endl; 
  
  if (SendHello() != 0) {
    close(socket_); 
    return 1; 
  }
  return 0; 
} 

int ClientWorker::HandleMoveTo(NodeData& nodeData) {
  int x = xPosition_->load();
  char* ip = nodeData.ipAddress; 
  int newPort = nodeData.port; 
  std::string action = nodeData.action; 
  //Fetch the destination
  size_t underscorePos = action.rfind('_'); 
  int destination = 0; 
  if (underscorePos != std::string::npos) {
    std::string arg = action.substr(underscorePos+1); 

    try{
      destination = std::stoi(arg); 
    }
    catch(std::invalid_argument e){
      std::cerr << "[Error] Received invalid positon" << std::endl; 
      return 1; 
    }
    catch(std::out_of_range e){
      std::cerr << "[Error] Received invalid positon" << std::endl;
      return 1;  
    }
  }

  //Convert port 
  porttype port; 
  if(newPort >= 0 && newPort <= std::numeric_limits<porttype>::max()) {
    port = static_cast<porttype>(newPort); 
  }
  else {
    std::cerr << "[ Node ] Received invalid port" << std::endl;
    return 1;  
  }

  SendOK();
  SimulateMovement(destination); 

  if(serverPort_.load() != port) {
    serverPort_.store(port); 
    if (Connect() != 0) {
      return 1; 
    }
  }

  return 0; 
}

int ClientWorker::HandleRemoveNode(NodeData& argData) {
  char* ip = argData.ipAddress; 
  int newPort = argData.port; 

  porttype port; 
  if(newPort >= 0 && newPort <= std::numeric_limits<porttype>::max()) {
    port = static_cast<porttype>(newPort); 
  }
  else {
    std::cerr << "[Error] Received invalid port" << std::endl;
    return 1;  
  }

  NodeData nodeData = { 0 }; 

  std::stringstream ss; 
  ss << actionToString(ActionType::REPLACE_SELF) << "_" << xPosition_->load();  
  std::strcpy(nodeData.action, ss.str().c_str()); 
  nodeData.nodeId = nodeId_->load();
  nodeData.port = serverPort_.load(); 

  int replacerId; 
  {
    const std::chrono::milliseconds timeout(5000); 
    auto deadline = std::chrono::steady_clock::now() + timeout;
    EnqueueInstruction(nodeData);  
    std::unique_lock<std::mutex> lock(*messageMutex_); 
    while ((replacerId = instructionFlag_->load()) == -2 && running_->load()){
      if (cv_->wait_until(lock, deadline) == std::cv_status::timeout) {
        std::cerr << "[Error] Remove node failure: Timed out." << std::endl;
        return 1; 
      }
    }
  }

  if (replacerId == -1) {
    std::cerr << "[Error] Remove node failure: Could not find replacement" << std::endl;
    return 1; 
  }
  instructionFlag_->store(-2);

  SimulateMovement(0); 

  xPosition_->store(0); 
  serverPort_.store(port); 

  if (Connect() != 0) {
    std::cerr << "[Error] Remove node failure: Failed to connect to main server" << std::endl;
    return 1; 
  }

  if (SendReplace(replacerId) != 0) {
    std::cerr << "[Error] Remove node warning: Failed to send replace" << std::endl;
    return 1; 
  }

  return 0; 
}

int ClientWorker::SendResponse(ActionType actionType) {
  NodeData nodeData = { 0 };
  std::string actionStr = actionToString(actionType);
  strcpy(nodeData.action, actionStr.c_str());
  nodeData.nodeId = nodeId_->load();
  nodeData.port = port_->load();

  int bytesSent = send(socket_, &nodeData, sizeof(nodeData), 0);
  if (bytesSent == -1) {
    std::cerr << "[Error] Failed to send data" << std::endl;
    return 1;
  }
  return 0;
}

int ClientWorker::SendHello() {
  return SendResponse(ActionType::HELLO); 
}

int ClientWorker::SendNone() {
  return SendResponse(ActionType::NONE); 
}

int ClientWorker::SendOK() {
  return SendResponse(ActionType::OK); 
}

int ClientWorker::SendReplace(int replacerId) {
  NodeData nodeData = { 0 };
  std::stringstream ss; 
  ss << actionToString(ActionType::REPLACE) << "_" << replacerId;
  std::string actionStr = ss.str();
  strcpy(nodeData.action, actionStr.c_str());
  nodeData.nodeId = nodeId_->load();
  nodeData.port = port_->load();

  int bytesSent = send(socket_, &nodeData, sizeof(nodeData), 0);
  if (bytesSent == -1) {
    std::cerr << "[Error] Failed to send data" << std::endl;
    return 1;
  }
  return 0;
}