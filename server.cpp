#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sstream>
#include <algorithm>
#include <deque>
#include <vector>
#include <map>
#include <utility>
#include <thread>
#include <mutex>
#include <chrono>
#include "npinc.h"
#include "nputility.hpp"
#include "message.hpp"

// mutex
struct Lockers {
    std::mutex accountLocker;
};

// account information
struct Account {
    std::string account;
    std::string password;
    bool isOnline;
    Account(const std::string& account, const std::string& password, const bool isOnline = false) :
        account(account), password(password), isOnline(isOnline) {}
};

// server utility
class ServerUtility {
public:
    ServerUtility(int fd) {
        this->fd = fd;
    }

    ~ServerUtility() {

    }

    void accountUtility(const std::string& msg) {
        std::lock_guard<std::mutex> lock(lockers.accountLocker);
        if (msg.find(msgREGISTER) == 0u) {
            accountRegister(msg);
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
            userData.insert(std::make_pair(account, Account(account, password)));
            std::string reply = msgSUCCESS;
            tcpWrite(fd, reply.c_str(), reply.length());
        }
    }

private:
    Lockers lockers;
    int fd;

private:
    std::map<std::string, Account> userData;
};

// thread related variables
bool isValid;
// vector of threads
std::vector<std::pair<std::thread, bool>> threads;
// thread related functions
void threadMaintain();
void finishThread();
std::string getThreadInfo();

// server init functions
int parseArgument(int argc, const char** argv);
int newServer(const int& port);
std::pair<sockaddr_in, int> newConnection(const int& listenfd);
std::pair<std::string, int> getConnectionInfo(const sockaddr_in sock);

// server main function
void serverFunc(int fd, std::pair<std::string, int> connectInfo);

int main(int argc, const char** argv) {
    int port = parseArgument(argc, argv);
    int listenfd = newServer(port);
    isValid = true;
    threads.push_back(std::make_pair(std::thread(threadMaintain), true));
    while (isValid) {
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
            if (std::string(command) == "q") {
                isValid = false;
                break;
            }
        }
        if (FD_ISSET(listenfd, &fdset)) {
            std::pair<sockaddr_in, int> client = newConnection(listenfd);
            std::pair<std::string, int> connectInfo = getConnectionInfo(client.first);
            threads.push_back(std::make_pair(std::thread(serverFunc, client.second, connectInfo), true));
        }
    }
    for (auto&& item : threads) {
        item.first.join();
    }
    return 0;
}

void threadMaintain() {
    while (isValid) {
        bool flag = false;
        unsigned toRemove = 0xFFFFFFFFu;
        for (unsigned i = 0; i < threads.size(); ++i) {
            if (!threads.at(i).second) {
                flag = true;
                toRemove = i;
            }
        }
        if (flag) {
            printf("Thread #%d finished.\n", toRemove);
            threads.at(toRemove).first.join();
            threads.erase(threads.begin() + toRemove);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void finishThread() {
    for (auto&& item : threads) {
        if (std::this_thread::get_id() == item.first.get_id()) {
            item.second = false;
            break;
        }
    }
}

std::string getThreadInfo() {
    std::ostringstream oss;
    oss << std::hex << std::this_thread::get_id();
    return oss.str();
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

int newServer(const int& port) {
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
        fprintf(stderr, "bind(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    listen(fd, 2048);
    return fd;
}

std::pair<sockaddr_in, int> newConnection(const int& listenfd) {
    sockaddr_in client;
    socklen_t clientlength = sizeof(client);
    int clientfd;
    if ((clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&client), &clientlength)) < 0) {
        fprintf(stderr, "accept(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return std::make_pair(client, clientfd);
}

std::pair<std::string, int> getConnectionInfo(const sockaddr_in sock) {
    std::string address = inet_ntoa(sock.sin_addr);
    int port = ntohs(sock.sin_port);
    return std::make_pair(address, port);
}

void serverFunc(int fd, std::pair<std::string, int> connectInfo) {
    printf("New thread id %s started\n", getThreadInfo().c_str());
    printf("New connection from %s port %d\n", connectInfo.first.c_str(), connectInfo.second);
    ServerUtility serverUtility(fd);
    int n;
    char buffer[MAXN];
    while (true) {
        if (tcpRead(fd, buffer, MAXN) <= 0) {
            break;
        }
        std::string command(buffer);
        if (command.find(msgREGISTER) == 0u) {
            serverUtility.accountUtility(command);
        }
    }
    finishThread();
    printf("Thread id %s finished\n", getThreadInfo().c_str());
    printf("%s port %d disconnected\n", connectInfo.first.c_str(), connectInfo.second);
}

