#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <deque>
#include <vector>
#include <map>
#include <utility>
#include <thread>
#include "Socket.hpp"
#include "npinc.h"

int main(const int argc, const char** argv) {
    if (argc != 3) {
        fprintf(stderr, "usage %s <server address> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int port;
    if (sscanf(argv[2], "%d", &port) != 1) {
        fprintf(stderr, "Invalid port number\n");
        return EXIT_FAILURE;
    }
    return 0;
}

