//
// Created by Kristian Vaula Jensen, May 2023
//
#pragma once
#ifndef MESH_NETWORK_CLIENTWORKER_H
#define MESH_NETWORK_CLIENTWORKER_H
#include "Worker.hpp"

typedef std::uint16_t porttype; 

class ClientWorker : public Worker{
  public: 
    ClientWorker(std::atomic<int>* nodeId, std::atomic<porttype>* port, std::atomic<int>* xPosition, std::atomic<bool>* instructionSucceeded,  std::queue<NodeData>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv); 
    ~ClientWorker(); 

    void RunClient(const std::string& serverPort); 

  private: 
    std::atomic<int>* xPosition_; 

    void HandleAction(NodeData& nodeData); 
    void SimulateMovement(int pos); 
    bool IsPassthrough(int nodeId); 
    int Connect(); 
    int HandleMoveTo(NodeData& nodeData); 
    int HandleRemoveNode(NodeData& nodeData); 
    int SendResponse(ActionType actionType);
    int SendHello();
    int SendNone();  
    int SendOK(); 
};

#endif //MESH_NETWORK_CLIENTWORKER_H