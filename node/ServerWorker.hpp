//
// Created by Kristian Vaula Jensen, May 2023
//
#pragma once
#ifndef MESH_NETWORK_SERVERWORKER_H
#define MESH_NETWORK_SERVERWORKER_H

#include "Worker.hpp"
#include <functional>
#include <set> 

typedef std::uint16_t porttype; 

class ServerWorker : public Worker {
  public:   
    ServerWorker(int* nodeId, int* port,std::queue<NodeData>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv); 
    ~ServerWorker();

    void RunServer(const std::string& serverPort); 

  private: 
    struct sockaddr_in serverAddress_;
    std::mutex clientSocketsMutex_; 
    std::set<int> connectedClientSockets_; 

    void BroadcastMessages(); 
    void HandleClient(int clientSocket); 
    int SetupServer(); 
};
#endif //MESH_NETWORK_SERVERWORKER_H