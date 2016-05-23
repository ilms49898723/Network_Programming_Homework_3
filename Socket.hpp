#ifndef NETWORK_PROGRAMMING_SOCKET_HPP_
#define NETWORK_PROGRAMMING_SOCKET_HPP_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>

class Socket;

class ServerSocket;

class Socket {
    friend class ServerSocket;

public:
    Socket() {
        memset(&sock, 0, sizeof(sock));
        this->address = "";
        this->port = -1;
        this->fd = -1;
    }

    Socket(const std::string& address, const int port, const int proto = AF_INET, const int type = SOCK_STREAM) {
        init(address, port, proto, type);
    }

    ~Socket() {

    }

    int init(const std::string& address, const int port, const int proto = AF_INET, const int type = SOCK_STREAM) {
        this->fd = -1;
        this->address = address;
        this->port = port;
        if ((this->fd = socket(proto, type, 0)) < 0) {
            fprintf(stderr, "Socket::init()::socket(): %s\n", strerror(errno));
            return -1;
        }
        memset(&sock, 0, sizeof(sock));
        sock.sin_family = proto;
        sock.sin_port = htons(port);
        if (inet_pton(proto, address.c_str(), &sock.sin_addr) <= 0) {
            fprintf(stderr, "Socket::init()::inet_pton(): %s\n", strerror(errno));
            return -1;
        }
        return 0;
    }

    int connect() {
        int result = ::connect(fd, reinterpret_cast<sockaddr*>(&sock), sizeof(sock));
        if (result < 0) {
            fprintf(stderr, "Socket::connect(): %s\n", strerror(errno));
            return -1;
        }
        return 0;
    }

    void close() const {
        if (fd >= 0) {
            ::close(fd);
        }
    }

    const std::string& getAddress() const {
        return address;
    }

    const int& getPort() const {
        return port;
    }

    const int& getfd() const {
        return fd;
    }

protected:
    sockaddr_in sock;
    std::string address;
    int port;
    int fd;
};

class ServerSocket {
private:
    constexpr static int MAXLISTENQ = 256;

public:
    ServerSocket() {
        memset(&sock, 0, sizeof(sock));
        this->port = -1;
        this->fd = -1;
    }

    ServerSocket(const int port, const int proto = AF_INET, const int type = SOCK_STREAM) {
        init(port, proto, type);
    }

    ~ServerSocket() {

    }

    int init(const int port, const int proto, const int type) {
        this->fd = -1;
        this->port = port;
        if ((this->fd = socket(proto, type, 0)) < 0) {
            fprintf(stderr, "ServerSocket::init()::socket(): %s\n", strerror(errno));
            return -1;
        }
        memset(&sock, 0, sizeof(sock));
        sock.sin_family = proto;
        sock.sin_addr.s_addr = htonl(INADDR_ANY);
        sock.sin_port = htons(port);
        if (bind(this->fd, reinterpret_cast<sockaddr*>(&sock), sizeof(sock)) < 0) {
            fprintf(stderr, "ServerSocket::init()::bind(): %s\n", strerror(errno));
            return -1;
        }
        listen(this->fd, MAXLISTENQ);
        return 0;
    }

    Socket accept() {
        Socket client;
        socklen_t socklength = sizeof(client.sock);
        if ((client.fd = ::accept(this->fd, reinterpret_cast<sockaddr*>(&client.sock), &socklength)) < 0) {
            fprintf(stderr, "ServerSocket()::accept(): %s\n", strerror(errno));
            return Socket();
        }
        printf("client.fd = %d\n", client.fd);
        client.port = ntohs(client.sock.sin_port);
        client.address = inet_ntoa(client.sock.sin_addr);
        return client;
    }

    void close() const {
        if (fd >= 0) {
            ::close(fd);
        }
    }

    const int& getPort() const {
        return port;
    }

    const int& getfd() const {
        return fd;
    }

private:
    sockaddr_in sock;
    int port;
    int fd;
};

#endif // NETWORK_PROGRAMMING_SOCKET_HPP_

