#ifndef NETWORK_PROGRAMMING_CLIENTUTILITY_HPP_
#define NETWORK_PROGRAMMING_CLIENTUTILITY_HPP_

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>
#include <deque>
#include <vector>
#include <map>
#include <utility>
#include <thread>
#include <mutex>
#include "npinc.hpp"
#include "nptype.hpp"
#include "nputility.hpp"
#include "message.hpp"
#include "ThreadUtil.hpp"

class ClientUtility {
private:
    constexpr static int MAXSPLITCHAR = 70;

public:
    ClientUtility(int fd, int p2pPort) {
        this->fd = fd;
        this->p2pPort = p2pPort;
        stage = NPStage::WELCOME;
    }

    ~ClientUtility() {

    }

    void printMessage(const std::string& msg, const bool isErrorMsg = false) {
        flushScreen();
        if (!isErrorMsg) {
            printf("\n%s\n\n", msg.c_str());
            lastmsg = msg;
        }
        else {
            printf("\n%s\n\n", lastmsg.c_str());
            printSplitLine();
            printf("%s\n", msg.c_str());
        }
        printSplitLine();
        switch (static_cast<int>(stage)) {
        case 0: // WELCOME
            printf("%s\n", optWELCOME.c_str());
            break;
        case 1: // MAIN
            printf("%s\n", optMAIN.c_str());
            break;
        default:
            break;
        }
    }

    void newAccount() {
        char account[MAXN];
        char password[MAXN];
        char confirmPassword[MAXN];
        printf("Account: ");
        if (fgets(account, MAXN, stdin) == NULL) {
            return;
        }
        trimNewLine(account);
        if (!isValidString(account)) {
            fprintf(stderr, "Account can not contain space, tabs\n");
            return;
        }
        strcpy(password, getpass("Password: "));
        trimNewLine(password);
        if (!isValidString(password)) {
            fprintf(stderr, "Password can not contain space, tabs\n");
            return;
        }
        strcpy(confirmPassword, getpass("Confirm password: "));
        trimNewLine(confirmPassword);
        if (!isValidString(confirmPassword)) {
            fprintf(stderr, "Password can not contain space, tabs\n");
            return;
        }
        if (strcmp(password, confirmPassword)) {
            fprintf(stderr, "Password not matched\n");
            return;
        }
        std::string msg = msgREGISTER + " " + account + " " + password;
        tcpWrite(fd, msg.c_str(), msg.length());
        char buffer[MAXN];
        tcpRead(fd, buffer, MAXN);
        printMessage(buffer);
    }

    void login() {
        char account[MAXN];
        char password[MAXN];
        printf("Account: ");
        if (fgets(account, MAXN, stdin) == NULL) {
            return;
        }
        trimNewLine(account);
        if (!isValidString(account)) {
            fprintf(stderr, "Account can not contain space, tabs\n");
            return;
        }
        strcpy(password, getpass("Password: "));
        trimNewLine(password);
        if (!isValidString(password)) {
            fprintf(stderr, "Password can not contain space, tabs\n");
            return;
        }
        std::string msg = msgLOGIN + " " + account + " " + password;
        tcpWrite(fd, msg.c_str(), msg.length());
        char buffer[MAXN];
        tcpRead(fd, buffer, MAXN);
        if (std::string(buffer).find(msgSUCCESS) == 0u) {
            stage = NPStage::MAIN;
            std::string info = msgUpdateConnectInfo + " " + account + " " + std::to_string(p2pPort);
            tcpWrite(fd, info.c_str(), info.length());
        }
        printMessage(buffer);
    }

public:
    void setStage(const NPStage stage) {
        this->stage = stage;
    }

    NPStage getStage() const {
        return stage;
    }

    void setp2pPort(const int p2pPort) {
        this->p2pPort = p2pPort;
    }

    int getp2pPort() const {
        return p2pPort;
    }

private:
    void flushScreen() {
        for (int i = 0; i < 50; ++i) {
            putc('\n', stdout);
        }
    }

    void printSplitLine() {
        for (int i = 0; i < MAXSPLITCHAR; ++i) {
            putc('-', stdout);
        }
        putc('\n', stdout);
    }

    bool isValidString(const std::string& str) {
        for (char c : str) {
            if (!isprint(c)) {
                return false;
            }
            if (c == ' ') {
                return false;
            }
        }
        return true;
    }

private:
    std::string lastmsg;
    NPStage stage;
    int fd;
    int p2pPort;
};
#endif // NETWORK_PROGRAMMING_CLIENTUTILITY_HPP_

