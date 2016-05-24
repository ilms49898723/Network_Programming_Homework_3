CC := g++

CXXFLAGS := -std=c++11 -Wall -Wextra -Os

DEP := Socket.hpp

.PHONY:
.PHONY: all server client clean

all: client server

client: client.cpp ${DEP}
	${CC} ${CXXFLAGS} ${DEP} $< -o $@ -pthread

server: server.cpp ${DEP}
	${CC} ${CXXFLAGS} ${DEP} $< -o $@ -pthread

Socket: ${DEP}
	${CC} ${CXXFLAGS} $< -c

clean:
	-rm -rf *.o client server

