CC := g++

CXXFLAGS := -std=c++11 -Wall -Wextra -Os -pthread

DEP := Socket.hpp

.PHONY:
.PHONY: all server client clean

all: client server

client: ${DEP}
	${CC} ${CXXFLAGS} ${DEP} client.cpp -o client

server: ${DEP}
	${CC} ${CXXFLAGS} ${DEP} server.cpp -o server

clean:
	-rm -rf *.o client server

