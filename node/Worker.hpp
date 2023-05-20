#pragma once

#include <iostream>
#include <string>
#include <cstring> 

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <thread>
#include <mutex>
#include <atomic>

typedef std::uint16_t porttype; 

class Worker {
  public: 
    Worker(); 
    virtual ~Worker(); 

    void SetServerport(const std::string& serverPort);
    void Stop(); 

  protected: 
    int socket_; 
    porttype serverPort_; 
    std::mutex mutex_; 
    std::atomic<bool> running_; 
}; 

Worker::Worker() : mutex_(), running_(true), socket_(-1) {}

Worker::~Worker() {
    Stop();
}

void Worker::SetServerport(const std::string& serverPort) {
  std::cout << "[Worker] Setting server port " << std::endl; 
  try {
    porttype port = static_cast<porttype>(std::stoi(serverPort)); 
    {
      std::unique_lock<std::mutex> lock(mutex_);
      this->serverPort_ = port; 
    }
  } catch (const std::invalid_argument& e) {
    std::cerr << "[Worker] Invalid port value: " << serverPort << std::endl; 
  } catch (const std::out_of_range& e) {
    std::cerr << "[Worker] Invalid port value: " << serverPort << std::endl; 
  }
}

void Worker::Stop() {
    running_.store(false);
}