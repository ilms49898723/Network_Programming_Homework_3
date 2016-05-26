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
        this->valid = true;
    }

    ~ServerUtility() {

    }

    bool isValid() const {
        return valid;
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
        else if (msg.find(msgDELETEACCOUNT) == 0u) {
            accountDelete(msg, data);
        }
        else if (msg.find(msgUPDATECONNECTINFO) == 0u) {
            accountUpdateConnectInfo(msg, data);
        }
        else if (msg.find(msgSHOWUSER) == 0u) {
            accountShowInfo(msg, data);
        }
        else if (msg.find(msgCHATREQUEST) == 0u) {
            accountSendConnectInfo(msg, data);
        }
    }

    void fileListUtility(const std::string& msg, ServerData& data) {
        std::lock_guard<std::mutex> lock(data.fileListLocker);
        if (msg.find(msgUPDATEFILELIST) == 0u) {
            fileListUpdate(msg, data);
        }
        else if (msg.find(msgSHOWFILELIST) == 0u) {
            fileListShow(msg, data);
        }
        else if (msg.find(msgGETFILELIST) == 0u) {
            fileListGet(msg, data);
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
            tcpWrite(fd, reply);
        }
        else {
            printLog("New account %s created\n", account);
            data.userData.insert(std::make_pair(account, Account(account, password)));
            std::string reply = msgSUCCESS;
            tcpWrite(fd, reply);
        }
    }

    // LOGIN account password
    void accountLogin(const std::string& msg, ServerData& data) {
        char account[MAXN];
        char password[MAXN];
        sscanf(msg.c_str() + msgLOGIN.length(), "%s%s", account, password);
        if (!data.userData.count(account) || data.userData.at(account).password != std::string(password)) {
            std::string reply = msgFAIL + " Invalid account or password";
            tcpWrite(fd, reply);
        }
        else if (data.userData.at(account).isOnline) {
            std::string reply = msgFAIL + " Already online, please log out first";
            tcpWrite(fd, reply);
        }
        else {
            nowAccount = account;
            data.userData[account].isOnline = true;
            std::string reply = msgSUCCESS;
            tcpWrite(fd, reply);
        }
    }

    // LOGOUT
    void accountLogout(const std::string& msg, ServerData& data) {
        if (msg.find(msgLOGOUT) != 0u || nowAccount == "") {
            return;
        }
        printLog("Account %s logout\n", nowAccount.c_str());
        data.userData[nowAccount].isOnline = false;
        nowAccount = "";
    }

    // DELETEACCOUNT
    void accountDelete(const std::string& msg, ServerData& data) {
        if (msg.find(msgDELETEACCOUNT) != 0u) {
            return;
        }
        data.userData.erase(nowAccount);
        for (auto& item : data.fileData) {
            item.second.owner.erase(nowAccount);
        }
        while (true) {
            bool flag = false;
            std::string filename;
            for (auto& item : data.fileData) {
                if (item.second.owner.empty()) {
                    filename = item.first;
                    flag = true;
                    break;
                }
            }
            if (flag) {
                data.fileData.erase(filename);
            }
            else {
                break;
            }
        }
        printLog("Account %s was deleted\n", nowAccount.c_str());
        nowAccount = "";
    }

    // UPDATECONNECTIONINFO port
    void accountUpdateConnectInfo(const std::string& msg, ServerData& data) {
        int port;
        sscanf(msg.c_str() + msgUPDATECONNECTINFO.length(), "%d", &port);
        data.userData[nowAccount].connectInfo = ConnectInfo(connectInfo.address, port);
        printLog("Account %s info updated IP %s port %d\n", nowAccount.c_str(), connectInfo.address.c_str(), port);
    }

    // SHOWUSER
    void accountShowInfo(const std::string& msg, ServerData& data) {
        if (msg.find(msgSHOWUSER) != 0u) {
            return;
        }
        std::string reply = "Online Users:\n";
        reply += "    Account                               IP                Port\n";
        for (auto& item : data.userData) {
            if (item.second.isOnline) {
                char formatBuffer[MAXN];
                snprintf(formatBuffer, MAXN, "    %-35s   %-15s   %d\n",
                         item.first.c_str(),
                         item.second.connectInfo.address.c_str(),
                         item.second.connectInfo.port);
                reply += formatBuffer;
            }
        }
        tcpWrite(fd, reply);
    }

    // CHATREQUEST account
    void accountSendConnectInfo(const std::string& msg, ServerData& data) {
        char account[MAXN];
        sscanf(msg.c_str() + msgCHATREQUEST.length(), "%s", account);
        if (!data.userData.count(account)) {
            std::string reply = msgFAIL + " User not found";
            tcpWrite(fd, reply);
        }
        else if (!data.userData[account].isOnline) {
            std::string reply = msgFAIL + " User is not online";
            tcpWrite(fd, reply);
        }
        else {
            std::string reply = msgSUCCESS;
            reply += " " + data.userData[account].connectInfo.address;
            reply += " " + std::to_string(data.userData[account].connectInfo.port);
            tcpWrite(fd, reply);
        }
    }

    // UPDATEFILELIST [files ...]
    void fileListUpdate(const std::string& msg, ServerData& data) {
        std::istringstream iss(msg.c_str() + msgUPDATEFILELIST.length());
        std::string filename;
        unsigned long filesize;
        printLog("Account %s files:\n", nowAccount.c_str());
        while (iss >> filename >> filesize) {
            data.fileData[filename].filename = filename;
            data.fileData[filename].size = filesize;
            data.fileData[filename].owner.insert(nowAccount);
            printf("          %s (%lu bytes)\n", filename.c_str(), filesize);
        }
    }

    // SHOWFILELIST
    void fileListShow(const std::string& msg, ServerData& data) {
        if (msg.find(msgSHOWFILELIST) != 0u) {
            return;
        }
        std::string reply = "Files:\n";
        for (auto& item : data.fileData) {
            reply += "    " + item.first + " (" + std::to_string(item.second.size) + " bytes)\n";
            for (auto& owners : item.second.owner) {
                reply += "        " + owners + ((data.userData[owners].isOnline) ? " [Online]" : " [Offline]") + "\n";
            }
        }
        tcpWrite(fd, reply);
    }

    // GETFILELIST account
    void fileListGet(const std::string& msg, ServerData& data) {
        char account[MAXN];
        sscanf(msg.c_str() + msgGETFILELIST.length(), "%s", account);
        if (!data.userData.count(account)) {
            std::string reply = msgFAIL + " User not found";
            tcpWrite(fd, reply);
        }
        else {
            std::string reply = std::string("Account: ") + account;
            reply += std::string(" ") + (data.userData[account].isOnline ? "[Online]" : "[Offline]") + "\n";
            for (auto& item : data.fileData) {
                if (item.second.owner.count(account)) {
                    reply += "    " + item.first + " (" + std::to_string(item.second.size) + " bytes)\n";
                }
            }
            tcpWrite(fd, reply);
        }
    }

private:
    std::string nowAccount;
    ConnectInfo connectInfo;
    int fd;
    bool valid;
};

#endif // NETWORK_PROGRAMMING_SERVERUTILITY_HPP_

