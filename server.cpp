#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <deque>
#include <vector>
#include <map>
#include <utility>
#include <thread>
#include <mutex>
#include <chrono>
#include "npinc.h"

std::vector<std::pair<std::thread, bool>> threads;

void threadMaintainer();
void serverFunc(int fd);
int parseArgument(int argc, const char** argv);
int newServer(const int& port);
std::pair<sockaddr_in, int> newConnection(const int& listenfd);

int main(int argc, const char** argv) {
    int port = parseArgument(argc, argv);
    int listenfd = newServer(port);
    threads.push_back(std::make_pair(std::thread(threadMaintainer), true));
    while (true) {
        std::pair<sockaddr_in, int> client = newConnection(listenfd);
        threads.push_back(std::make_pair(std::thread(serverFunc, client.second), true));
    }
    return 0;
}

void threadMaintainer() {
    while (true) {
        bool flag = false;
        unsigned toRemove = 0xFFFFFFFFu;
        for (unsigned i = 0; i < threads.size(); ++i) {
            if (!threads.at(i).second) {
                flag = true;
                toRemove = i;
            }
        }
        if (flag) {
            printf("Remove thread #%d(finished)\n", toRemove);
            threads.at(toRemove).first.join();
            threads.erase(threads.begin() + toRemove);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void serverFunc(int fd) {
    char buffer[MAXN];
    while (true) {
        int n;
        if ((n = read(fd, buffer, MAXN)) == 0) {
            break;
        }
        if (n < 0) {
            fprintf(stderr, "%s", strerror(errno));
            break;
        }
        printf("%s", buffer);
        if ((n = write(fd, buffer, MAXN)) == 0) {
            break;
        }
        if (n < 0) {
            fprintf(stderr, "%s", strerror(errno));
            break;
        }
    }
    for (auto&& i : threads) {
        if (std::this_thread::get_id() == i.first.get_id()) {
            printf("Marked\n");
            i.second = false;
            break;
        }
    }
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

