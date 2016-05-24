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

void clientFunc(const std::pair<sockaddr_in, int>& server);
std::pair<std::string, int> parseArgument(const int& argc, const char**& argv);
std::pair<sockaddr_in, int> newConnection(const std::pair<std::string, int>& connectInfo);

int main(int argc, const char** argv) {
    std::pair<std::string, int> connectInfo = parseArgument(argc, argv);
    std::pair<sockaddr_in, int> server = newConnection(connectInfo);
    clientFunc(server);
    return 0;
}

void clientFunc(const std::pair<sockaddr_in, int>& server) {
    const int fd = server.second;
    char buffer[MAXN];
    while (fgets(buffer, MAXN, stdin)) {
        if (write(fd, buffer, MAXN) < 0) {
            fprintf(stderr, "%s\n", strerror(errno));
            break;
        }
        if (read(fd, buffer, MAXN) < 0) {
            fprintf(stderr, "%s\n", strerror(errno));
            break;
        }
        printf("recv %s", buffer);
    }
}

std::pair<std::string, int> parseArgument(const int& argc, const char**& argv) {
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
    return std::make_pair(address, port);
}

std::pair<sockaddr_in, int> newConnection(const std::pair<std::string, int>& connectInfo) {
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(connectInfo.second);
    if (inet_pton(AF_INET, connectInfo.first.c_str(), &server.sin_addr) < 0) {
        fprintf(stderr, "inet_pton(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (connect(fd, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
        fprintf(stderr, "connect(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return std::make_pair(server, fd);
}

