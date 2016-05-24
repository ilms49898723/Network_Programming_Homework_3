#ifndef NETWORK_PROGRAMMING_NPTYPE_HPP_
#define NETWORK_PROGRAMMING_NPTYPE_HPP_

#include <string>

enum class NPStage {
    WELCOME, MAIN
};

struct ConnectData {
    sockaddr_in sock;
    int fd;
    ConnectData() {
        memset(&sock, 0, sizeof(sock));
        fd = -1;
    }
    ConnectData(const sockaddr_in& sock, const int fd) :
        sock(sock), fd(fd) {}
};

struct ConnectInfo {
    std::string address;
    int port;
    ConnectInfo(const std::string& address = "", const int port = -1) :
        address(address), port(port) {}
};

#endif // NETWORK_PROGRAMMING_NPTYPE_HPP_

