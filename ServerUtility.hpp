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

    void accountUtility(const std::string& msg, ServerData& data) {
        std::lock_guard<std::mutex> lock(data.accountLocker);
        if (msg.find(msgREGISTER) == 0u) {
            accountRegister(msg, data);
        }
        else if (msg.find(msgLOGIN) == 0u) {
            accountLogin(msg, data);
        }
        else if (msg.find(msgLOGOUT) == 0u) {
            accountLogout(msg, data);
        }
        else if (msg.find(msgUPDATECONNECTINFO) == 0u) {
            accountUpdateConnectInfo(msg, data);
        }
    }

    void fileListUtility(const std::string& msg, ServerData& data) {
        std::lock_guard<std::mutex> lock(data.fileListLocker);
        if (msg.find(msgUPDATEFILELIST) == 0u) {
            fileUpdateList(msg, data);
        }
    }

private:
    // REGISTER account password
    void accountRegister(const std::string& msg, ServerData& data) {
        char account[MAXN];
        char password[MAXN];
        sscanf(msg.c_str() + msgREGISTER.length(), "%s%s", account, password);
        if (data.userData.count(account)) {
            std::string reply = msgFAIL + " Account already exists";
            tcpWrite(fd, reply.c_str(), reply.length());
        }
        else {
            printLog("New account %s created\n", account);
            data.userData.insert(std::make_pair(account, Account(account, password)));
            std::string reply = msgSUCCESS;
            tcpWrite(fd, reply.c_str(), reply.length());
        }
    }

    // LOGIN account password
    void accountLogin(const std::string& msg, ServerData& data) {
        char account[MAXN];
        char password[MAXN];
        sscanf(msg.c_str() + msgLOGIN.length(), "%s%s", account, password);
        if (!data.userData.count(account) || data.userData.at(account).password != std::string(password)) {
            std::string reply = msgFAIL + " Invalid account or password";
            tcpWrite(fd, reply.c_str(), reply.length());
        }
        else {
            nowAccount = account;
            data.userData[account].isOnline = true;
            std::string reply = msgSUCCESS;
            tcpWrite(fd, reply.c_str(), reply.length());
        }
    }

    // LOGOUT
    void accountLogout(const std::string& msg, ServerData& data) {
        if (msg.find(msgLOGOUT) != 0u) {
            return;
        }
        printLog("Account %s logout\n", nowAccount.c_str());
        data.userData[nowAccount].isOnline = false;
        nowAccount = "";
    }

    // UPDATECONNECTIONINFO port
    void accountUpdateConnectInfo(const std::string& msg, ServerData& data) {
        int port;
        sscanf(msg.c_str() + msgUPDATECONNECTINFO.length(), "%d", &port);
        data.userData[nowAccount].connectInfo = ConnectInfo(connectInfo.address, port);
        printLog("Account %s info updated IP %s port %d\n", nowAccount.c_str(), connectInfo.address.c_str(), port);
    }

    // UPDATEFILELIST [files ...]
    void fileUpdateList(const std::string& msg, ServerData& data) {
        std::istringstream iss(msg.c_str() + msgUPDATEFILELIST.length());
        std::string filename;
        printLog("Account %s files:\n", nowAccount.c_str());
        while (iss >> filename) {
            data.fileData[filename].insert(nowAccount);
            printf("          %s\n", filename.c_str());
        }
    }

private:
    std::string nowAccount;
    ConnectInfo connectInfo;
    int fd;
};

#endif // NETWORK_PROGRAMMING_SERVERUTILITY_HPP_

