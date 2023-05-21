#pragma once

#include <iostream>
#include <string>
#include <cstring> 
#include <queue>
#include <chrono>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

typedef std::uint16_t porttype; 

class Worker {
  public: 
    Worker(std::queue<std::string>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv); 
    virtual ~Worker(); 

    void EnqueueInstruction(const std::string& message); 
    void SetServerport(const std::string& serverPort);
    void Stop(); 

  protected: 
    int socket_; 
    porttype serverPort_; 
    std::mutex mutex_; 
    std::atomic<bool> running_; 

    std::queue<std::string> &messageQueue_; 
    std::mutex* messageMutex_; 
    std::condition_variable* cv_; 
}; 
/*
Constructor
*/
Worker::Worker(std::queue<std::string>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv) 
  : messageQueue_(messageQueue), messageMutex_(messageMutex), cv_(cv), mutex_(), running_(true), socket_(-1) {}

/*
Deconstructor
*/
Worker::~Worker() {
    Stop();
}

/*
Adds an instruction to the queue
*/
void Worker::EnqueueInstruction(const std::string& instruction) {
  {
    std::unique_lock<std::mutex> lock(*messageMutex_); 
    messageQueue_.push(instruction); 
  }
  cv_->notify_one(); 
}

/*
Sets the server port of the worker
*/
void Worker::SetServerport(const std::string& serverPort) {
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

/*
Changes running_ to false to stop running thread
*/
void Worker::Stop() {
    running_.store(false);
}