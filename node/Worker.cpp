//
// Created by Kristian Vaula Jensen, May 2023
//
#include "Worker.hpp"
/*
Constructor
*/
Worker::Worker(std::atomic<int>* nodeId, std::atomic<porttype>* port, std::atomic<bool>* running, std::atomic<int>* instructionFlag, std::queue<NodeData>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv) 
  : nodeId_(nodeId), port_(port), instructionFlag_(instructionFlag), messageQueue_(messageQueue), messageMutex_(messageMutex), cv_(cv), workerMutex_(), running_(running), socket_(-1) {}

/*
Deconstructor
*/
Worker::~Worker() {
    Stop();
}

/*
Adds an instruction to the queue
*/
void Worker::EnqueueInstruction(const NodeData& instruction) {
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
      std::unique_lock<std::mutex> lock(workerMutex_);
      this->serverPort_ = port; 
    }
  } catch (const std::invalid_argument& e) {
    std::cerr << "[Error] Invalid port value: " << serverPort << std::endl; 
  } catch (const std::out_of_range& e) {
    std::cerr << "[Error] Invalid port value: " << serverPort << std::endl; 
  }
}

/*
Changes running_ to false to stop running thread
*/
void Worker::Stop() {
    running_->store(false);
    cv_->notify_all(); 
}