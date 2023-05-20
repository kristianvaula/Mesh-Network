#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class ClientWorker {
  public: 
    ClientWorker(); 
    ~ClientWorker(); 

    void Run(const std::string& serverPort); 
    void Stop(); 
    void SetServerport(const std::string& serverPort); 
  private: 
    std::string serverPort_; 
    std::mutex mutex_; 
    std::atomic<bool> running_; 
};

ClientWorker::ClientWorker() : serverPort_(""), mutex_(), running_(true) {
}

ClientWorker::~ClientWorker() {
  Stop();
}

void ClientWorker::Run(const std::string& serverPort) {
  std::cout << "Running client" << std::endl; 
  running_.store(true); 
  SetServerport(serverPort);  

  while (running_.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(300)); 
  }

  std::cout << "Stopping client" << std::endl; 
}

void ClientWorker::Stop() {
  running_.store(false); 
}

void ClientWorker::SetServerport(const std::string& serverPort) {
  try {
    int port = std::stoi(serverPort); 
    if (port >= 0 && port <= 65535) {
      {
        std::unique_lock<std::mutex> lock(mutex_);
        this->serverPort_ = serverPort; 
      }
    }
    else {
      std::cerr << "Invalid port value: " << serverPort << std::endl; 
    }
  } catch (const std::invalid_argument& e) {
    std::cerr << "Invalid port value: " << serverPort << std::endl; 
  } catch (const std::out_of_range& e) {
    std::cerr << "Invalid port value: " << serverPort << std::endl; 
  }
}

