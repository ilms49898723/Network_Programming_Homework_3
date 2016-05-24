#ifndef NETWORK_PROGRAMMING_NPUTILITY_HPP
#define NETWORK_PROGRAMMING_NPUTILITY_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>
#include <string>
#include "npinc.h"

void setSocketTimeout(const int& socketfd, const int& second, const int& millisecond) {
    timeval tv;
    tv.tv_sec = second;
    tv.tv_usec = millisecond * 1000;
    if (setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        fprintf(stderr, "setsockopt: %s\n", strerror(errno));
    }
    tv.tv_sec = second;
    tv.tv_usec = millisecond * 1000;
    if (setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
        fprintf(stderr, "setsockopt: %s\n", strerror(errno));
    }
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

std::pair<sockaddr_in, int> newClient(const int& listenfd) {
    sockaddr_in client;
    socklen_t clientlength = sizeof(client);
    int clientfd;
    if ((clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&client), &clientlength)) < 0) {
        fprintf(stderr, "accept(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return std::make_pair(client, clientfd);
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

std::pair<std::string, int> getConnectionInfo(const sockaddr_in sock) {
    std::string address = inet_ntoa(sock.sin_addr);
    int port = ntohs(sock.sin_port);
    return std::make_pair(address, port);
}

int tcpWrite(const int fd, const char* msg, const size_t n) {
    char buffer[MAXN];
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, msg, n);
    int m = write(fd, buffer, MAXN);
    if (m < 0) {
        fprintf(stderr, "write: %s\n", strerror(errno));
    }
    return m;
}

int tcpRead(const int fd, char* buffer, const size_t n) {
    memset(buffer, 0, sizeof(char) * n);
    int m = read(fd, buffer, n);
    if (m < 0) {
        fprintf(stderr, "read: %s\n", strerror(errno));
    }
    return m;
}

char* trimNewLine(char* src) {
    if (src[strlen(src) - 1] == '\n') {
        src[strlen(src) - 1] = '\0';
    }
    return src;
}

char* toLowerString(char* src) {
    for (char* ptr = src; *ptr; ++ptr) {
        *ptr = tolower(*ptr);
    }
    return src;
}

#endif //NETWORK_PROGRAMMING_NPUTILITY_HPP
