#include "ClientWorker.hpp"

ClientWorker::ClientWorker(int* nodeId, int* port, std::atomic<int>* xPosition, std::queue<NodeData>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv) 
: xPosition_(xPosition), Worker(nodeId, port, messageQueue, messageMutex,cv){
}

ClientWorker::~ClientWorker() {}

void ClientWorker::RunClient(const std::string& serverPort) {
  running_.store(true); 

  SetServerport(serverPort);  
  if (Connect() != 0) {
    return; 
  }

  if (SendHello() != 0) {
    return; 
  } 

  NodeData nodeData = { 0 }; 
  int bytesRead; 
  bool passthrough; 

  while (running_.load()) {
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
    else {
      std::cout << "[ClientWorker] Receiveed empty server message, closing socket" << std::endl;
      close(socket_); 
      break;  
    }
  }
  close(socket_);  
}

void ClientWorker::HandleAction(NodeData& nodeData) {
  ActionType action = actionFromString(nodeData.action); 
  switch (action) {
    case (ActionType::HELLO): // Hello 
      std::cout << "[Server] Hello" << std::endl; 
      break; 
    case (ActionType::MOVETO):// Move to <x> command 
      std::cout << "[Server] Move To" << std::endl;
      HandleMoveTo(nodeData); 
      break; 
    case (ActionType::REMOVE_NODE): 
      std::cout << "[Server] Remove Node" << std::endl;
      SendOK(); 
      break; 

    case (ActionType::REPLACE): 
      std::cout << "[Server] Replace" << std::endl;
      SendOK(); 
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
  std::cout << "[Node] Moving to " << pos << "."; 
  for (uint8_t i = 0; i < 3; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "."; 
  }
  std::cout << std::endl; 
}

bool ClientWorker::IsPassthrough(int nodeId) {
  bool result = false; 
  {
    std::unique_lock<std::mutex> lock(workerMutex_);
    if (nodeId != *nodeId_) {
      result = true; 
    } 
  }
  return result; 
}

int ClientWorker::Connect() {
  porttype serverPort; 
  {
    std::unique_lock<std::mutex> lock(workerMutex_); 
    serverPort = serverPort_; 
  }

  socket_ = socket(AF_INET, SOCK_STREAM, 0); 
  if (socket_ == -1) {
    std::cout << "[ClientWorker] Failed to connect to parent, returning" << std::endl;
    return 1;
  }

  //Consider adding options 

  struct sockaddr_in serverAddress; 
  serverAddress.sin_family = AF_INET; 
  serverAddress.sin_port = htons(serverPort); 
  if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0) {
    std::cerr << "[ClientWorker] Failed to convert address" << std::endl; 
    close(socket_); 
    return 1;  
  }

  if (connect(socket_, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    std::cerr << "[ClientWorker] Failed to connect to the server" << std::endl; 
    close(socket_); 
    return 1; 
  }

  std::cout << "[ClientWorker] Connected to server" << std::endl; 
  return 0; 
} 

int ClientWorker::HandleMoveTo(NodeData& nodeData) {
  int x = xPosition_->load();
  char* ip = nodeData.ipAddress; 
  int newPort = nodeData.port; 
  std::string action = nodeData.action; 
  std::cout << "Action: " << action << std::endl; 
  //Fetch the destination
  size_t underscorePos = action.find('_'); 
  int destination; 
  if (underscorePos != std::string::npos) {
    std::string arg = action.substr(underscorePos+1); 
    std::cout << "Arg: " << arg << std::endl; 

    try{
      destination = std::stoi(arg); 
    }
    catch(std::invalid_argument e){
      std::cerr << "[ClientWorker] Received invalid positon" << std::endl; 
      return 1; 
    }
    catch(std::out_of_range e){
      std::cerr << "[ClientWorker] Received invalid positon" << std::endl;
      return 1;  
    }
  }

  //Convert port 
  porttype port; 
  if(newPort >= 0 && newPort <= std::numeric_limits<porttype>::max()) {
    port = static_cast<porttype>(newPort); 
  }
  else {
    std::cerr << "[ClientWorker] Received invalid port" << std::endl;
    return 1;  
  }

  SendOK();
  close(socket_); 

  //Simulate movement 
  SimulateMovement(destination); 
  {
    std::unique_lock<std::mutex> lock(workerMutex_); 
    xPosition_->store(destination); 
    serverPort_ = port; 
  }
  if (Connect() != 0) {
    return 1; 
  }
  return 0; 
}

int ClientWorker::SendResponse(ActionType actionType) {
  NodeData nodeData = { 0 };
  std::string actionStr = actionToString(actionType);
  strcpy(nodeData.action, actionStr.c_str());
  {
    std::unique_lock<std::mutex> lock(workerMutex_);
    nodeData.nodeId = *nodeId_;
    nodeData.port = *port_;
  }
  int bytesSent = send(socket_, &nodeData, sizeof(nodeData), 0);
  if (bytesSent == -1) {
    std::cerr << "[ClientWorker] Failed to send data" << std::endl;
    return 1;
  }
  return 0;
}

int ClientWorker::SendHello() {
  return SendResponse(ActionType::HELLO); 
}

int ClientWorker::SendOK() {
  return SendResponse(ActionType::OK); 
}
