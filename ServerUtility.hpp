#ifndef NETWORK_PROGRAMMING_SERVERUTILITY_HPP_
#define NETWORK_PROGRAMMING_SERVERUTILITY_HPP_

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>
#include <deque>
#include <vector>
#include <map>
#include <utility>
#include <thread>
#include <mutex>
#include "npinc.hpp"
#include "nptype.hpp"
#include "nputility.hpp"
#include "message.hpp"
#include "ThreadUtil.hpp"

class ServerUtility {
public:
    ServerUtility(int fd, ConnectInfo connectInfo) {
        this->fd = fd;
        this->connectInfo = connectInfo;
    }

    ~ServerUtility() {

    }

    void accountUtility(const std::string& msg) {
        std::lock_guard<std::mutex> lock(accountLocker);
        if (msg.find(msgREGISTER) == 0u) {
            accountRegister(msg);
        }
        else if (msg.find(msgLOGIN) == 0u) {
            accountLogin(msg);
        }
        else if (msg.find(msgUpdateConnectInfo) == 0u) {
            accountUpdateConnectInfo(msg);
        }
    }

private:
    // REGISTER account password
    void accountRegister(const std::string& msg) {
        char account[MAXN];
        char password[MAXN];
        sscanf(msg.c_str() + msgREGISTER.length(), "%s%s", account, password);
        if (userData.count(account)) {
            std::string reply = msgFAIL + " Account already exists";
            tcpWrite(fd, reply.c_str(), reply.length());
        }
        else {
            printLog("New account %s created\n", account);
            userData.insert(std::make_pair(account, Account(account, password)));
            std::string reply = msgSUCCESS;
            tcpWrite(fd, reply.c_str(), reply.length());
        }
    }

    // LOGIN account password
    void accountLogin(const std::string& msg) {
        char account[MAXN];
        char password[MAXN];
        sscanf(msg.c_str() + msgLOGIN.length(), "%s%s", account, password);
        if (!userData.count(account) || userData.at(account).password != std::string(password)) {
            std::string reply = msgFAIL + " Invalid account or password";
            tcpWrite(fd, reply.c_str(), reply.length());
        }
        else {
            userData[account].isOnline = true;
            std::string reply = msgSUCCESS;
            tcpWrite(fd, reply.c_str(), reply.length());
        }
    }

    // UPDATECONNECTIONINFO account port
    void accountUpdateConnectInfo(const std::string& msg) {
        char account[MAXN];
        int port;
        sscanf(msg.c_str() + msgUpdateConnectInfo.length(), "%s%d", account, &port);
        userData[account].connectInfo = ConnectInfo(connectInfo.address, port);
        printLog("Account %s connection info updated. IP %s port %d\n", account, connectInfo.address.c_str(), port);
    }

private:
    std::mutex accountLocker;
    ConnectInfo connectInfo;
    int fd;

private:
    std::map<std::string, Account> userData;
};

#endif // NETWORK_PROGRAMMING_SERVERUTILITY_HPP_

