#include <iostream>
#include <string>
#include <cstring> 
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef std::uint16_t porttype; 

class ClientWorker {
  public: 
    ClientWorker(); 
    ~ClientWorker(); 

    void Run(const std::string& serverPort); 
    void Stop(); 
    void SetServerport(const std::string& serverPort); 
  private: 
    int socket_; 
    porttype serverPort_; 
    std::mutex mutex_; 
    std::atomic<bool> running_; 

    int Connect(); 
};

ClientWorker::ClientWorker() : mutex_(), running_(true), socket_(-1){
}

ClientWorker::~ClientWorker() {
  Stop();
}

void ClientWorker::Run(const std::string& serverPort) {
  std::cout << "[ClientWorker] Running client" << std::endl; 

  running_.store(true); 

  SetServerport(serverPort);  
  if (Connect() == -1) {
    return; 
  }

  char buffer[1024] = { 0 }; 
  int bytesRead, bytesSent; 
  std::string hello = "Hello from client";

  bytesSent = send(socket_, hello.c_str(), hello.length(), 0); 
  if (bytesSent <= 0) {
    std::cout << "[ClientWorker] failed to send hello" << std::endl; 
  }

  while (running_.load()) {
    bytesRead = read(socket_, buffer, 1024); 
    if(bytesRead > 0) {
      std::cout << buffer << std::endl; 
      memset(buffer, 0, sizeof(buffer)); 
    }
    else {
      std::cout << "[ClientWorker] Receiveed empty server message, closing socket" << std::endl;
      close(socket_); 
      break;  
    }
    
    //Receive string requests  
    //Convert into struct NodeData in InterpretRequest()
    //Handle them in HandleRequest()
    //Get response with CreateResponse()
    //Send response; 
    
  }

  std::cout << "[ClientWorker] Stopping client" << std::endl; 
}

void ClientWorker::Stop() {
  running_.store(false); 
}

void ClientWorker::SetServerport(const std::string& serverPort) {
  try {
    porttype port = static_cast<porttype>(std::stoi(serverPort)); 
    {
      std::unique_lock<std::mutex> lock(mutex_);
      this->serverPort_ = port; 
    }
  } catch (const std::invalid_argument& e) {
    std::cerr << "[ClientWorker] Invalid port value: " << serverPort << std::endl; 
  } catch (const std::out_of_range& e) {
    std::cerr << "[ClientWorker] Invalid port value: " << serverPort << std::endl; 
  }
}

int ClientWorker::Connect() {
  porttype serverPort; 
  {
    std::unique_lock<std::mutex> lock(mutex_); 
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
