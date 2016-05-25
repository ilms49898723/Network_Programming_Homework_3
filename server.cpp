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

// server init functions
int parseArgument(int argc, const char** argv);
// server main function
void serverFunc(const int fd, ConnectInfo connectInfo);

// server utility
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

int main(int argc, const char** argv) {
    lb::setLogEnabled(true);
    lb::threadManageInit();
    int port = parseArgument(argc, argv);
    int listenfd = newServer(port);
    while (lb::isValid()) {
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(fileno(stdin), &fdset);
        FD_SET(listenfd, &fdset);
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200000;
        int nready = select(std::max(fileno(stdin), listenfd) + 1, &fdset, NULL, NULL, &tv);
        if (nready < 0) {
            if (errno == EINTR) {
                continue;
            }
            fprintf(stderr, "select(): %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(fileno(stdin), &fdset)) {
            char command[MAXN];
            if (fgets(command, MAXN, stdin) == NULL) {
                exit(EXIT_FAILURE);
            }
            trimNewLine(command);
            toLowerString(command);
            if (std::string(command) == "q" || std::string(command) == "quit") {
                lb::setValid(false);
                break;
            }
        }
        if (FD_ISSET(listenfd, &fdset)) {
            ConnectData client = newClient(listenfd);
            ConnectInfo connectInfo = getConnectInfo(client.sock);
            lb::pushThread(std::thread(serverFunc, client.fd, connectInfo));
        }
    }
    lb::joinAll();
    return 0;
}

int parseArgument(int argc, const char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port;
    if (sscanf(argv[1], "%d", &port) != 1) {
        fprintf(stderr, "%s: not a valid port number\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    return port;
}

void serverFunc(const int fd, ConnectInfo connectInfo) {
    printLog("New connection from %s port %d\n", connectInfo.address.c_str(), connectInfo.port);
    ServerUtility serverUtility(fd, connectInfo);
    char buffer[MAXN];
    while (true) {
        if (tcpRead(fd, buffer, MAXN) <= 0) {
            break;
        }
        std::string command(buffer);
        if (command.find(msgREGISTER) == 0u) {
            serverUtility.accountUtility(command);
        }
        else if (command.find(msgLOGIN) == 0u) {
            serverUtility.accountUtility(command);
        }
        else if (command.find(msgUpdateConnectInfo) == 0u) {
            serverUtility.accountUtility(command);
        }
    }
    lb::finishThread();
    printLog("%s port %d disconnected\n", connectInfo.address.c_str(), connectInfo.port);
}

