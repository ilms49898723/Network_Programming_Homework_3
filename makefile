CC := g++

CXXFLAGS := -std=c++11 -Wall -Wextra -Os -pthread

DEP := Socket.hpp

.PHONY:
.PHONY: all server client clean

all: client server

client: client.cpp ${DEP}
	${CC} ${CXXFLAGS} ${DEP} $< -o $@

server: server.cpp ${DEP}
	${CC} ${CXXFLAGS} ${DEP} $< -o $@

Socket: ${DEP}
	${CC} ${CXXFLAGS} $< -c

clean:
	-rm -rf *.o client server

