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

class ClientUtility {
public:
    ClientUtility(int fd) {
        this->fd = fd;
    }

    ~ClientUtility() {

    }

    void accountRegister() {
        char account[MAXN];
        char password[MAXN];
        char confirmPassword[MAXN];
        printf("Account: ");
        if (fgets(account, MAXN, stdin) == NULL) {
            return;
        }
        trimNewLine(account);
        if (!isValidString(account)) {
            fprintf(stderr, "Account can not contain space, tabs\n");
            return;
        }
        strcpy(password, getpass("Password: "));
        trimNewLine(password);
        if (!isValidString(password)) {
            fprintf(stderr, "Password can not contain space, tabs\n");
            return;
        }
        strcpy(confirmPassword, getpass("Confirm password: "));
        trimNewLine(confirmPassword);
        if (!isValidString(confirmPassword)) {
            fprintf(stderr, "Password can not contain space, tabs\n");
            return;
        }
        if (strcmp(password, confirmPassword)) {
            fprintf(stderr, "Password not matched\n");
            return;
        }
        std::string msg = msgREGISTER + " " + account + " " + password;
        tcpWrite(fd, msg.c_str(), msg.length());
    }

private:
    bool isValidString(const std::string& str) {
        for (char c : str) {
            if (!isprint(c)) {
                return false;
            }
            if (c == ' ') {
                return false;
            }
        }
        return true;
    }

private:
    int fd;
};

// thread related variables
bool isValid;
std::mutex threadLocker;
// vector of threads
std::vector<std::pair<std::thread, bool>> threads;
// thread related functions
void threadMaintain();
void finishThread();
void joinAll();
std::string getThreadId();

// client variables
NPStage stage;
ConnectionInfo myself;
// client init functions
ConnectionInfo parseArgument(const int& argc, const char**& argv);
// client main functions
void printMessage(const std::string& msg = "");
void clientFunc(const ConnectionData& server);
void clientRecv(const int fd);
// p2p server for file transfer
void p2pserverInit();
void p2pserverAccept(const int listenfd);
void p2pserverFunc(int fd, ConnectionInfo connectInfo);

int main(int argc, const char** argv) {
    threadLocker.lock();
    threads.push_back(std::make_pair(std::thread(threadMaintain), true));
    threadLocker.unlock();
    isValid = true;
    stage = NPStage::WELCOME;
    ConnectionInfo connectInfo = parseArgument(argc, argv);
    ConnectionData server = newConnection(connectInfo);
    p2pserverInit();
    clientFunc(server);
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

std::string getThreadId() {
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

void printMessage(const std::string& msg) {
    for (int i = 0; i < 50; ++i) {
        putc('\n', stdout);
    }
    printf("\n%s\n\n", msg.c_str());
    switch (static_cast<int>(stage)) {
        case 0: // WELCOME
            printf("%s\n", optWELCOME.c_str());
            break;
        default:
            break;
    }
}

void clientFunc(const ConnectionData& server) {
    threadLocker.lock();
    threads.push_back(std::make_pair(std::thread(clientRecv, server.fd), true));
    threadLocker.unlock();
    ClientUtility clientUtility(server.fd);
    char buffer[MAXN];
    printMessage("Welcome!!!");
    while (true) {
        if (fgets(buffer, MAXN, stdin) == NULL) {
            break;
        }
        trimNewLine(buffer);
        toUpperString(buffer);
        std::string command(buffer);
        if (command == "Q" || command == "QUIT") {
            break;
        }
        switch (static_cast<int>(stage)) {
            case 0: // WELCOME
                if (command == "R") {
                    clientUtility.accountRegister();
                }
                else if (command == "L") {

                }
                break;
            default:
                break;
        }
    }
}

void clientRecv(const int fd) {
    while (isValid) {
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200000;
        int nready = select(fd + 1, &fdset, NULL, NULL, &tv);
        if (nready < 0) {
            if (errno == EINTR) {
                continue;
            }
            fprintf(stderr, "select: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(fd, &fdset)) {
            char buffer[MAXN];
            if (tcpRead(fd, buffer, MAXN) < 0) {
                break;
            }
            printMessage(buffer);
        }
    }
    finishThread();
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
    std::lock_guard<std::mutex> lock(threadLocker);
    threads.push_back(std::make_pair(std::thread(p2pserverAccept, serverfd), true));
}

void p2pserverAccept(const int listenfd) {
    while (isValid) {
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(listenfd, &fdset);
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200000;
        int nready = select(listenfd + 1, &fdset, NULL, NULL, &tv);
        if (nready < 0) {
            if (errno == EINTR) {
                continue;
            }
            fprintf(stderr, "select: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(listenfd, &fdset)) {
            ConnectionData client = newClient(listenfd);
            ConnectionInfo connectInfo = getConnectionInfo(client.sock);
            std::lock_guard<std::mutex> lock(threadLocker);
            threads.push_back(std::make_pair(std::thread(p2pserverFunc, client.fd, connectInfo), true));
        }
    }
    finishThread();
}

void p2pserverFunc(int fd, ConnectionInfo connectInfo) {
    finishThread();
}

