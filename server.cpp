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
    if (argc != 2) {
        fprintf(stderr, "usage %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int port;
    if (sscanf(argv[1], "%d", &port) != 1) {
        fprintf(stderr, "Invalid port number\n");
        return EXIT_FAILURE;
    }
    return 0;
}

