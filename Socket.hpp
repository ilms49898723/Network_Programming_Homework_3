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
        if (this->fd >= 0) {
            close(this->fd);
        }
    }

    int init(const std::string& address, const int port, const int proto = AF_INET, const int type = SOCK_STREAM) {
        this->fd = -1;
        this->address = address;
        this->port = port;
        if ((this->fd = socket(proto, type, 0)) < 0) {
            return errno;
        }
        memset(&sock, 0, sizeof(sock));
        sock.sin_family = proto;
        sock.sin_port = htons(port);
        if (inet_pton(proto, address.c_str(), &sock.sin_addr) <= 0) {
            return errno;
        }
    }

    int connect() {
        int result = ::connect(fd, reinterpret_cast<sockaddr*>(&sock), sizeof(sock));
        if (result < 0) {
            return errno;
        }
        return 0;
    }

    const std::string& getAddress() const {
        return this->address;
    }

    const int& getPort() const {
        return this->port;
    }

    const int& getfd() const {
        return fd;
    }

private:
    sockaddr_in sock;
    std::string address;
    int port;
    int fd;
};

class ServerSocket {
private:
    constexpr static int MAXLISTENQ = 2048;

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
        if (this->fd >= 0) {
            close(this->fd);
        }
    }

    int init(const int port, const int proto, const int type) {
        this->fd = -1;
        this->port = port;
        memset(&sock, 0, sizeof(sock));
        sock.sin_family = proto;
        sock.sin_addr.s_addr = htonl(INADDR_ANY);
        sock.sin_port = htons(port);
        if ((this->fd = socket(proto, type, 0)) < 0) {
            return errno;
        }
        if (bind(this-fd, reinterpret_cast<sockaddr*>(&sock), sizeof(sockaddr_in)) < 0) {
            return errno;
        }
        listen(this->fd, MAXLISTENQ);
    }

    Socket accept() {
        socklen_t socklength = sizeof(client.sock);
        Socket client;
        if ((client.fd = ::accept(this->fd, reinterpret_cast<sockaddr*>(&client.sock), &socklength)) < 0) {
            return Socket();
        }
        client.port = ntohs(client.sock.sin_port);
        client.address = inet_ntoa(client.sin_addr);
        return client;
    }

    const int& getPort() const {
        return this->port;
    }

    const int& getFd() const {
        return this->fd;
    }

private:
    sockaddr_in sock;
    int port;
    int fd;
};

#endif // NETWORK_PROGRAMMING_SOCKET_HPP_

