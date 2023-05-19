#ifndef MESH_NETWORK_IP_UTILS_H
#define MESH_NETWORK_IP_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class IpUtils {
public:
    IpUtils();
    void check_host_name(int hostname);
    void check_host_entry(struct hostent* hostentry);
    void getIPAddress(char* ipAddressBuffer, int bufferSize);
};

#endif // MESH_NETWORK_IP_UTILS_H
