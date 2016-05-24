#ifndef NETWORK_PROGRAMMING_UDPUTIL_HPP_
#define NETWORK_PROGRAMMING_UDPUTIL_HPP_

#include <cstdio>
#include <cstring>
#include <memory>
#include "nputility.hpp"
#include "npinc.h"

class UDPSeqCounter {
public:
    UDPSeqCounter() {
        init();
    }

    ~UDPSeqCounter() {

    }

    void init() {
        seq = 0;
    }

    void incSeq() {
        seq++;
    }

    void setSeq(const unsigned long long& seq) {
        this->seq = seq;
    }

    unsigned long long getSeq() const {
        return seq;
    }

private:
    unsigned long long seq;
};

class UDPUtil {
public:
    constexpr static int PMAXN = 2200;

public:
    unsigned long long lastSeq;

public:
    UDPUtil() {
        cnt.init();
        lastSeq = 0;
    }

    ~UDPUtil() {

    }

    int udpTrans(int fd, sockaddr*& sockp, char* dst, size_t dn, const char* src, size_t sn) {
        // init timeout
        setSocketTimeout(fd, 0, 100);
        socklen_t sockLen = sizeof(*sockp);
        unsigned long long thisSeq = cnt.getSeq();
        cnt.incSeq();
        char toSend[PMAXN];
        char toRecv[PMAXN];
        memset(toSend, 0, sizeof(toSend));
        memcpy(toSend, src, sn);
        pack(toSend, thisSeq);
        int byteSend;
        int counter;
        counter = 0;
        while ((byteSend = sendto(fd, toSend, sn + 20, 0, sockp, sockLen)) < 0) {
            counter++;
            if (counter > 3) {
                fprintf(stderr, "UDPUtil: udpTrans: sendto: %s\n",strerror(errno));
                return -1;
            }
        }
        int byteRecv;
        counter = 0;
        while (true) {
            if (counter < 10) {
                setSocketTimeout(fd, 0, 100); // init
            }
            else if (counter < 15) {
                setSocketTimeout(fd, 0, 200); // retry 10 times
            }
            else if (counter < 20) {
                setSocketTimeout(fd, 0, 500); // retry 15 times
            }
            else {
                setSocketTimeout(fd, 0, 800); // maybe timeout
            }
            memset(toRecv, 0, sizeof(toRecv));
            byteRecv = recvfrom(fd, toRecv, PMAXN, 0, sockp, &sockLen);
            ++counter;
            if (counter > 25) {
                fprintf(stderr, "Timeout! Can\'t connect to server or server is too busy\n");
                fprintf(stderr, "Please try again later\n");
                return -1;
            }
            if (byteRecv < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    bool flag = false;
                    for (int i = 0; i < 3; ++i) {
                        if ((byteSend = sendto(fd, toSend, sn + 20, 0, sockp, sockLen)) >= 0) {
                            flag = true;
                            break;
                        }
                    }
                    if (!flag) {
                        fprintf(stderr, "UDPUtil: udpTrans: sendto: %s\n", strerror(errno));
                        return -1;
                    }
                }
                else {
                    fprintf(stderr, "UDPUtil: udpTrans: recvfrom: %s\n", strerror(errno));
                    return -1;
                }
            }
            else if (byteRecv < 20) {
                byteSend = sendto(fd, toSend, sn + 20, 0, sockp, sockLen);
            }
            else {
                unsigned long long seq = getSeq(toRecv);
                if (seq == thisSeq) {
                    unpack(toRecv);
                    memset(dst, 0, sizeof(char) * dn);
                    memcpy(dst, toRecv, dn);
                    dst[byteRecv - 20] = '\0';
                    return byteRecv - 20;
                }
            }
        }
    }

    int udpSend(int fd, sockaddr*& sockp, const char* src, size_t n) {
        socklen_t sockLen = sizeof(*sockp);
        char temp[PMAXN];
        memset(temp, 0, sizeof(temp));
        memcpy(temp, src, n);
        pack(temp, lastSeq);
        int byteSend = sendto(fd, temp, n + 20, 0, sockp, sockLen);
        if (byteSend < 0) {
            fprintf(stderr, "UDPUtil: udpSend: sendto: %s\n", strerror(errno));
        }
        return byteSend;
    }

    int udpRecv(int fd, sockaddr*& sockp, char* dst, size_t n) {
        socklen_t sockLen = sizeof(*sockp);
        char temp[PMAXN];
        int byteRecv = recvfrom(fd, temp, PMAXN, 0, sockp, &sockLen);
        if (byteRecv < 0) {
            fprintf(stderr, "UDPUtil: udpRecv: recvfrom: %s\n", strerror(errno));
            return -1;
        }
        if (byteRecv < 20) {
            fprintf(stderr, "UDPUtil: udpRecv: imcomplete packet\n");
            return -1;
        }
        lastSeq = getSeq(temp);
        unpack(temp);
        memcpy(dst, temp, n);
        dst[byteRecv - 20] = '\0';
        return byteRecv - 20;
    }

private:
    unsigned long long getSeq(const char* src) {
        char tmpStr[20];
        memset(tmpStr, 0, sizeof(tmpStr));
        memcpy(tmpStr, src + 4, 16);
        unsigned long long ret;
        sscanf(tmpStr, "%llx", &ret);
        return ret;
    }

    void pack(char* src, unsigned long long seq) {
        char prefix[] = "\x02\x02";
        char postfix[] = "\x03\x03";
        char tmp[PMAXN];
        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, prefix, 2);
        snprintf(tmp + 2, PMAXN - 2, "%016llx", seq);
        memcpy(tmp + 18, postfix, 2);
        memcpy(tmp + 20, src, PMAXN - 20);
        memset(src, 0, sizeof(char) * PMAXN);
        memcpy(src, tmp, PMAXN);
    }

    void unpack(char* src) {
        char tmp[PMAXN];
        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, src + 20, PMAXN - 20);
        memset(src, 0, sizeof(char) * PMAXN);
        memcpy(src, tmp, PMAXN);
    }

private:
    UDPSeqCounter cnt;
};

#endif // NETWORK_PROGRAMMING_UDPUTIL_HPP_

