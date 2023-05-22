//
// Created by Kristian Vaula Jensen, May 2023
//
#pragma once

#ifndef MESH_NETWORK_WORKER_H
#define MESH_NETWORK_WORKER_H

#include <iostream>
#include <string>
#include <cstring> 
#include <cerrno>
#include <limits>
#include <queue>
#include <vector> 
#include <chrono>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h> 
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "../model/dto/SocketData.hpp"
#include "../model/enums/ActionType.hpp"

typedef std::uint16_t porttype; 

class Worker {
  public: 
    Worker(std::atomic<int>* nodeId, std::atomic<porttype>* port, std::atomic<bool>* running, std::atomic<bool>* instructionSucceeded, std::queue<NodeData>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv); 
    virtual ~Worker(); 

    void EnqueueInstruction(const NodeData& message); 
    void SetServerport(const std::string& serverPort);
    void Stop(); 

  protected: 
    int socket_; 
    std::atomic<porttype> serverPort_; 
    std::mutex workerMutex_; 

    std::atomic<int>* nodeId_; 
    std::atomic<porttype>* port_;
    std::atomic<bool>* running_; 

    std::atomic<bool>* instructionSucceeded_; 
    std::queue<NodeData>& messageQueue_; 
    std::mutex* messageMutex_; 
    std::condition_variable* cv_; 
}; 

#endif //MESH_NETWORK_WORKER_H