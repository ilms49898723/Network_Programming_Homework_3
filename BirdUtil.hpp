#ifndef NETWORK_PROGRAMMING_BIRDUTIL_HPP_
#define NETWORK_PROGRAMMING_BIRDUTIL_HPP_

#include <cstring>

void trimNewLine(char* src) {
    if (src[strlen(src) - 1] == '\n') {
        src[strlen(src) - 1] = '\0';
    }
}

#endif // NETWORK_PROGRAMMING_BIRDUTIL_HPP_

