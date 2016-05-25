#ifndef NETWORK_PROGRAMMING_NPTYPE_HPP_
#define NETWORK_PROGRAMMING_NPTYPE_HPP_

#include <string>
#include <vector>
#include <map>

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

// account information
struct Account {
    std::string account;
    std::string password;
    std::vector<std::string> files;
    ConnectInfo connectInfo;
    bool isOnline;
    Account(const std::string& account = "", const std::string& password = "", const bool isOnline = false) :
        account(account), password(password), isOnline(isOnline) {}
};

struct ServerData {
    std::map<std::string, Account> userData;
};

#endif // NETWORK_PROGRAMMING_NPTYPE_HPP_

