//
// Created by Kristian Vaula Jensen, May 2023
//
#pragma once

#ifndef MESH_NETWORK_WORKER_H
#define MESH_NETWORK_WORKER_H

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

#include "../model/dto/SocketData.h"
#include "../model/enums/ActionType.hpp"

typedef std::uint16_t porttype; 

class Worker {
  public: 
    Worker(int* nodeId, int* port, std::queue<NodeData>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv); 
    virtual ~Worker(); 

    void EnqueueInstruction(const NodeData& message); 
    void SetServerport(const std::string& serverPort);
    void Stop(); 

  protected: 
    int* nodeId_; 
    int* port_;
    int socket_; 
    porttype serverPort_; 
    std::mutex workerMutex_; 
    std::atomic<bool> running_; 

    std::queue<NodeData>& messageQueue_; 
    std::mutex* messageMutex_; 
    std::condition_variable* cv_; 
}; 

#endif //MESH_NETWORK_WORKER_H