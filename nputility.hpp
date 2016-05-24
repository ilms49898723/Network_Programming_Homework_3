#ifndef NETWORK_PROGRAMMING_NPUTILITY_HPP
#define NETWORK_PROGRAMMING_NPUTILITY_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
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

void trimNewLine(char* src) {
    if (src[strlen(src) - 1] == '\n') {
        src[strlen(src) - 1] = '\0';
    }
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

#endif //NETWORK_PROGRAMMING_NPUTILITY_HPP
