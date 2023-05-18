#ifndef MESH_NETWORK_SOCKET_DATA_H
#define MESH_NETWORK_SOCKET_DATA_H

#include <string>

struct NodeData {
    int nodeId;
    int port;
    char action[256];
};

#endif // MESH_NETWORK_SOCKET_DATA_H