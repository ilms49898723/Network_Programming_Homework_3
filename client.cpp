#include <cstdio>
#include <cstring>
#include <cstdlib>
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

// thread related variables
bool isValid;
std::mutex threadLocker;
// vector of threads
std::vector<std::pair<std::thread, bool>> threads;
// thread related functions
void threadMaintain();
void finishThread();
void joinAll();
std::string getThreadInfo();

// client init functions
ConnectionInfo parseArgument(const int& argc, const char**& argv);
// client main functions
void clientFunc(const ConnectionData& server);
void clientRecv(const int fd);
void p2pserverInit();
void p2pserverAccept(const int listenfd);
void p2pserverFunc(int fd, ConnectionInfo connectInfo);

int main(int argc, const char** argv) {
    threadLocker.lock();
    threads.push_back(std::make_pair(std::thread(threadMaintain), true));
    threadLocker.unlock();
    isValid = true;
    ConnectionInfo connectInfo = parseArgument(argc, argv);
    ConnectionData server = newConnection(connectInfo);
    clientFunc(server);
    isValid = false;
    joinAll();
    return 0;
}

void threadMaintain() {
    while (isValid) {
        threadLocker.lock();
        bool flag = true;
        while (flag) {
            flag = false;
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
        }
        threadLocker.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void finishThread() {
    std::lock_guard<std::mutex> lock(threadLocker);
    for (auto& item : threads) {
        if (std::this_thread::get_id() == item.first.get_id()) {
            item.second = false;
            break;
        }
    }
}

void joinAll() {
    isValid = false;
    for (auto& item : threads) {
        if (item.first.joinable()) {
            item.first.join();
        }
    }
}

std::string getThreadInfo() {
    std::ostringstream oss;
    oss << std::hex << std::this_thread::get_id();
    return oss.str();
}

ConnectionInfo parseArgument(const int& argc, const char**& argv) {
    if (argc != 3) {
        fprintf(stderr, "usage %s <server address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    std::string address;
    int port;
    address = argv[1];
    if (sscanf(argv[2], "%d", &port) != 1) {
        fprintf(stderr, "%s: not a valid port number\n", argv[2]);
        exit(EXIT_FAILURE);
    }
    return ConnectionInfo(address, port);
}

void clientFunc(const ConnectionData& server) {
    threadLocker.lock();
    threads.push_back(std::make_pair(std::thread(clientRecv, server.fd), true));
    threadLocker.unlock();
    const int fd = server.fd;
    std::string msg = msgNEWCONNECTION;
    tcpWrite(fd, msg.c_str(), msg.length());
    char buffer[MAXN];
    while (fgets(buffer, MAXN, stdin)) {
        trimNewLine(buffer);
        toLowerString(buffer);
        std::string command(buffer);
        if (command == "q" || command == "quit") {
            break;
        }
    }
}

void clientRecv(const int fd) {
    char buffer[MAXN];
    if (tcpRead(fd, buffer, MAXN) < 0) {
        return;
    }
    printf("\n%s\n", buffer);
}

void p2pserverInit() {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::minstd_rand randomGenerator(seed);
    int port;
    int serverfd;
    while (true) {
        port = randomGenerator() % 1000 + 50000;
        if ((serverfd = newServer(port)) >= 0) {
            break;
        }
    }
}

void p2pserverAccept(const int listenfd) {
    while (isValid) {
        ConnectionData client = newClient(listenfd);
        ConnectionInfo connectInfo = getConnectionInfo(client.sock);
        std::lock_guard<std::mutex> lock(threadLocker);
        threads.push_back(std::make_pair(std::thread(p2pserverFunc, client.fd, connectInfo), true));
    }
}

void p2pserverFunc(int fd, ConnectionInfo connectInfo) {

}

