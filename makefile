CC := g++

CXXFLAGS := -std=c++11 -Wall -Wextra -Os

DEP := nputility.hpp UDPUtil.hpp

.PHONY:
.PHONY: all server client clean

all: client server

client: client.cpp
	${CC} ${CXXFLAGS} ${DEP} $< -o $@ -pthread

server: server.cpp
	${CC} ${CXXFLAGS} ${DEP} $< -o $@ -pthread

clean:
	-rm -rf *.o client server

