#ifndef MESH_NETWORK_SOCKET_DATA_HPP
#define MESH_NETWORK_SOCKET_DATA_HPP

#include <string>

struct NodeData {
    int nodeId;
    int port;
    char ipAddress[256];
    char action[256];
};

#endif // MESH_NETWORK_SOCKET_DATA_HPP