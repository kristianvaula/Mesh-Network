#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>

typedef std::uint16_t porttype;

void HandleClient(int clientSocket) {
  char buffer[1024] = {0};
  int bytesRead; 
  while ((bytesRead = read(clientSocket, buffer, 1024)) > 0) {
    std::cout << "[Server] Received message from client: " << buffer << std::endl;
    std::string response = "Hello from testserver";
    int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
    if (bytesSent <= 0) {
      std::cerr << "[Server] Failed to send response" << std::endl;
    }
    memset(buffer, 0, sizeof(buffer));
  }
  close(clientSocket);
}

int main() {
  porttype serverPort = 12345;

  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
    std::cerr << "[Server] Failed to create server socket" << std::endl;
    return 1;
  }

  struct sockaddr_in serverAddress{};
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(serverPort);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    std::cerr << "[Server] Failed to bind server socket" << std::endl;
    close(serverSocket);
    return 1;
  }

  if (listen(serverSocket, 5) < 0) {
    std::cerr << "[Server] Failed to listen on server socket" << std::endl;
    close(serverSocket);
    return 1;
  }

  std::cout << "[Server] Listening on port " << serverPort << std::endl;

  while (true) {
    struct sockaddr_in clientAddress{};
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
    if (clientSocket < 0) {
      std::cerr << "[Server] Failed to accept client connection" << std::endl;
      continue;
    }
    std::cout << "[Server] Accepted client connection" << std::endl;

    std::thread clientThread(HandleClient, clientSocket);
    clientThread.detach();
  }

  close(serverSocket);
  return 0;
}