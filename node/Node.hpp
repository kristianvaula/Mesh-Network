#include "ClientWorker.hpp"
#include "ServerWorker.hpp"

typedef std::uint16_t porttype; 

class Node {
  public: 
    Node(int id, porttype port, std::atomic<int>* instructionFlag, std::queue<NodeData>& messageQueue, std::mutex* messageMutex, std::condition_variable* cv); 
    ~Node(); 

    void Stop(); 
    void StartClient(const std::string& serverPort); 
    void StartServer();
    void StopClient(); 
    void StopServer(); 
    void ServerThreadMethod(const std::string &serverPort); 
    void ClientThreadMethod(const std::string &serverPort); 
    bool IsClientRunning() const; 
    bool IsServerRunning() const; 
 
  private: 
    std::atomic<int> id_;
    std::atomic<porttype> port_; 
    std::atomic<int> xPosition_; 
    std::thread serverThread_; 
    std::thread clientThread_; 
    ServerWorker serverWorker_; 
    ClientWorker clientWorker_; 
    std::atomic<bool> clientRunning_; 
    std::atomic<bool> serverRunning_; 

};