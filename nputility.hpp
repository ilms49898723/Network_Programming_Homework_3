#ifndef NETWORK_PROGRAMMING_NPUTILITY_HPP_
#define NETWORK_PROGRAMMING_NPUTILITY_HPP_

#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>
#include <string>
#include "npinc.hpp"
#include "nptype.hpp"

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
        return -1;
    }
    sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
        fprintf(stderr, "bind(): %s\n", strerror(errno));
        return -1;
    }
    listen(fd, 2048);
    return fd;
}

ConnectData newClient(const int& listenfd) {
    sockaddr_in client;
    socklen_t clientlength = sizeof(client);
    int clientfd;
    if ((clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&client), &clientlength)) < 0) {
        fprintf(stderr, "accept(): %s\n", strerror(errno));
        return ConnectData();
    }
    return ConnectData(client, clientfd);
}

ConnectData newConnection(const ConnectInfo& connectInfo) {
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket(): %s\n", strerror(errno));
        return ConnectData();
    }
    sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(connectInfo.port);
    if (inet_pton(AF_INET, connectInfo.address.c_str(), &server.sin_addr) < 0) {
        fprintf(stderr, "inet_pton(): %s\n", strerror(errno));
        return ConnectData();
    }
    if (connect(fd, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
        fprintf(stderr, "connect(): %s\n", strerror(errno));
        return ConnectData();
    }
    return ConnectData(server, fd);
}

ConnectInfo getConnectInfo(const sockaddr_in sock) {
    std::string address = inet_ntoa(sock.sin_addr);
    int port = ntohs(sock.sin_port);
    return ConnectInfo(address, port);
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

void printLog(const char* format, ...) {
    va_list args;
    va_start(args, format);
    time_t now = time(NULL);
    char timeBuffer[MAXN];
    strftime(timeBuffer, MAXN, "[%T]", localtime(&now));
    printf("%s", timeBuffer);
    vprintf(format, args);
    va_end(args);
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

char* toUpperString(char* src) {
    for (char* ptr = src; *ptr; ++ptr) {
        *ptr = toupper(*ptr);
    }
    return src;
}

#endif //NETWORK_PROGRAMMING_NPUTILITY_HPP_

