#ifndef NETWORK_PROGRAMMING_NPTYPE_HPP_
#define NETWORK_PROGRAMMING_NPTYPE_HPP_

#include <string>

struct ConnectionData {
    sockaddr_in sock;
    int fd;
    ConnectionData() {
        memset(&sock, 0, sizeof(sock));
        fd = -1;
    }
    ConnectionData(const sockaddr_in& sock, const int fd) :
        sock(sock), fd(fd) {}
};

struct ConnectionInfo {
    std::string address;
    int port;
    ConnectionInfo(const std::string& address = "", const int port = -1) :
        address(address), port(port) {}
};

#endif // NETWORK_PROGRAMMING_NPTYPE_HPP_

