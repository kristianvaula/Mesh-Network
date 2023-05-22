#include "IpUtils.hpp"

IpUtils::IpUtils() {}

void IpUtils::check_host_name(int hostname) {
    if (hostname == -1) {
        perror("gethostname");
        exit(1);
    }
}

void IpUtils::check_host_entry(struct hostent* hostentry) {
    if (hostentry == NULL) {
        perror("gethostbyname");
        exit(1);
    }
}

void IpUtils::getIPAddress(char* ipAddressBuffer, int bufferSize) {
    char host[256];
    struct hostent* host_entry;
    int hostname;

    hostname = gethostname(host, sizeof(host));
    check_host_name(hostname);
    host_entry = gethostbyname(host);
    check_host_entry(host_entry);

    const char* ipAddress = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));

    strncpy(ipAddressBuffer, ipAddress, bufferSize - 1);
    ipAddressBuffer[bufferSize - 1] = '\0';
}
