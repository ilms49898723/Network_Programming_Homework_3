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
public:
    ClientUtility(int fd, int p2pPort, int row, int col) {
        this->fd = fd;
        this->p2pPort = p2pPort;
        this->terminalRow = row;
        this->terminalCol = col;
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
            printf("%s\n$ ", optWELCOME.c_str());
            break;
        case 1: // MAIN
            printf("%s\n%s:$ ", optMAIN.c_str(), nowAccount.c_str());
            break;
        default:
            break;
        }
        fflush(stdout);
    }

    void printPrevious() {
        printMessage(lastmsg);
    }

public:
    void newAccount() {
        char account[MAXN];
        char password[MAXN];
        char confirmPassword[MAXN];
        printf("Account: ");
        if (fgets(account, MAXN, stdin) == NULL) {
            printPrevious();
            return;
        }
        trimNewLine(account);
        if (!isValidString(account)) {
            printMessage("Account cannot contain space, tabs", true);
            return;
        }
        strcpy(password, getpass("Password: "));
        trimNewLine(password);
        if (!isValidString(password)) {
            printMessage("Password cannot contain space, tabs", true);
            return;
        }
        strcpy(confirmPassword, getpass("Confirm password: "));
        trimNewLine(confirmPassword);
        if (!isValidString(confirmPassword)) {
            printMessage("Password cannot contain space, tabs", true);
            return;
        }
        if (strcmp(password, confirmPassword)) {
            printMessage("Password not matched", true);
            return;
        }
        std::string msg = msgREGISTER + " " + account + " " + password;
        tcpWrite(fd, msg.c_str(), msg.length());
        char buffer[MAXN];
        tcpRead(fd, buffer, MAXN);
        printMessage(buffer);
    }

    void deleteAccount() {
        char buffer[MAXN];
        printf("Are you sure?(yes/no) ");
        if (fgets(buffer, MAXN, stdin) == NULL) {
            printPrevious();
            return;
        }
        trimNewLine(buffer);
        if (std::string(buffer) == "yes") {
            std::string info = msgDELETEACCOUNT;
            tcpWrite(fd, info.c_str(), info.length());
            stage = NPStage::WELCOME;
            nowAccount = "";
            printMessage("Success!");
        }
        else {
            printMessage("Canceled", true);
        }
    }

    void login() {
        char account[MAXN];
        char password[MAXN];
        printf("Account: ");
        if (fgets(account, MAXN, stdin) == NULL) {
            printPrevious();
            return;
        }
        trimNewLine(account);
        if (!isValidString(account)) {
            printMessage("Account cannot contain space, tabs", true);
            return;
        }
        strcpy(password, getpass("Password: "));
        trimNewLine(password);
        if (!isValidString(password)) {
            printMessage("Password cannot contain space, tabs", true);
            return;
        }
        std::string msg = msgLOGIN + " " + account + " " + password;
        tcpWrite(fd, msg.c_str(), msg.length());
        char buffer[MAXN];
        tcpRead(fd, buffer, MAXN);
        if (std::string(buffer).find(msgSUCCESS) == 0u) {
            stage = NPStage::MAIN;
            nowAccount = account;
            updateConnectInfo();
            updateFileList();
            printMessage(std::string("Login Success!\n\nWelcome ") + account + "!");
        }
        else {
            printMessage(buffer);
        }
    }

    void logout() {
        if (nowAccount == "") {
            return;
        }
        std::string info = msgLOGOUT;
        tcpWrite(fd, info.c_str(), info.length());
        nowAccount = "";
        stage = NPStage::WELCOME;
        printMessage("Logout Success");
    }

    void updateConnectInfo() {
        std::string info = msgUPDATECONNECTINFO + " " + std::to_string(p2pPort);
        tcpWrite(fd, info.c_str(), info.length());
    }

    void updateFileList() {
        DIR* dir = opendir("Client");
        if (!dir) {
            printMessage("Cannot open directory Client", true);
            return;
        }
        dirent *dirst;
        std::string info = msgUPDATEFILELIST;
        std::string msg;
        while ((dirst = readdir(dir))) {
            if (dirst->d_type == DT_DIR) {
                continue;
            }
            std::string filepath = std::string("Client/") + dirst->d_name;
            struct stat st;
            if (stat(filepath.c_str(), &st) < 0) {
                continue;
            }
            info += std::string(" ") + std::string(dirst->d_name) + std::string(" ") + std::to_string(st.st_size);
            msg += "    " + std::string(dirst->d_name) + "\n";
        }
        closedir(dir);
        tcpWrite(fd, info.c_str(), info.length());
        printMessage(std::string("File List updated!\n") + msg);
    }

    void showFileList() {
        std::string info = msgSHOWFILELIST;
        tcpWrite(fd, info.c_str(), info.length());
        char buffer[MAXN];
        tcpRead(fd, buffer, MAXN);
        printMessage(buffer);
    }

    void showUser() {
        std::string info = msgSHOWUSER;
        tcpWrite(fd, info.c_str(), info.length());
        char buffer[MAXN];
        tcpRead(fd, buffer, MAXN);
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
        for (int i = 0; i < terminalRow; ++i) {
            putc('\n', stdout);
        }
    }

    void printSplitLine() {
        for (int i = 0; i < terminalCol; ++i) {
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
    std::string nowAccount;
    std::string lastmsg;
    NPStage stage;
    int fd;
    int p2pPort;

private:
    int terminalRow;
    int terminalCol;
};

#endif // NETWORK_PROGRAMMING_CLIENTUTILITY_HPP_

