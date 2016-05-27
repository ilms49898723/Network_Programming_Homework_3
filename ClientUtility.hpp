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
    ClientUtility() {
        this->fd = -1;
        this->p2pPort = -1;
        this->terminalRow = -1;
        this->terminalCol = -1;
        this->needUpdateDir = false;
        stage = NPStage::WELCOME;
    }

    ~ClientUtility() {

    }

    void init(int fd, int p2pPort, int row, int col) {
        this->fd = fd;
        this->p2pPort = p2pPort;
        this->terminalRow = row;
        this->terminalCol = col;
        stage = NPStage::WELCOME;
    }

    void printMessage(const std::string& msg, const bool isNotification = false) {
        flushScreen();
        if (!isNotification) {
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
            printf("%s%s%s\n", COLOR_BRIGHT_BLUE, optWELCOME.c_str(), COLOR_NORMAL);
            printf("%sGUEST%s$ ", COLOR_BRIGHT_GREEN, COLOR_NORMAL);
            break;
        case 1: // MAIN
            printf("%s%s%s\n", COLOR_BRIGHT_BLUE, optMAIN.c_str(), COLOR_NORMAL);
            printf("%s%s%s$ ", COLOR_BRIGHT_GREEN, nowAccount.c_str(), COLOR_NORMAL);
            break;
        default:
            break;
        }
    }

    void printPrevious() {
        printMessage(lastmsg);
    }

    void setNeedUpdateDir() {
        needUpdateDir = true;
    }

    std::string getLastmsg() const {
        return lastmsg;
    }

public:
    bool checkConnection() {
        char buffer[MAXN];
        tcpWrite(fd, msgCHECKCONNECT);
        tcpRead(fd, buffer, MAXN);
        return std::string(buffer) == msgCHECKCONNECT;
    }

    bool checkConnection(int specfd) {
        char buffer[MAXN];
        tcpWrite(specfd, msgCHECKCONNECT);
        tcpRead(specfd, buffer, MAXN);
        return std::string(buffer) == msgCHECKCONNECT;
    }

    void updateDir() {
        if (needUpdateDir && nowAccount != "") {
            updateFileList(true);
        }
        needUpdateDir = false;
    }

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
        tcpWrite(fd, msg);
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
            tcpWrite(fd, info);
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
        tcpWrite(fd, msg);
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
        tcpWrite(fd, info);
        nowAccount = "";
        stage = NPStage::WELCOME;
        printMessage("Logout Success");
    }

    void updateConnectInfo() {
        std::string info = msgUPDATECONNECTINFO + " " + std::to_string(p2pPort);
        tcpWrite(fd, info);
    }

    void updateFileList(bool silent = false) {
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
            msg += "    " + std::string(dirst->d_name) + " (" + std::to_string(st.st_size) + " bytes)\n";
        }
        closedir(dir);
        tcpWrite(fd, info);
        if (!silent) {
            printMessage(std::string("File List updated!\n") + msg);
        }
    }

    void getFileList() {
        char account[MAXN];
        printf("Account: ");
        if (fgets(account, MAXN, stdin) == NULL) {
            printPrevious();
            return;
        }
        trimNewLine(account);
        std::string msg = msgGETFILELIST + " " + account;
        tcpWrite(fd, msg);
        char buffer[MAXN];
        tcpRead(fd, buffer, MAXN);
        if (std::string(buffer).find(msgFAIL) == 0u) {
            printMessage(buffer, true);
        }
        else {
            printMessage(buffer);
        }
    }

    void showFileList() {
        std::string info = msgSHOWFILELIST;
        tcpWrite(fd, info);
        char buffer[MAXN];
        tcpRead(fd, buffer, MAXN);
        printMessage(buffer);
    }

    void showUser() {
        std::string info = msgSHOWUSER;
        tcpWrite(fd, info);
        char buffer[MAXN];
        tcpRead(fd, buffer, MAXN);
        printMessage(buffer);
    }

    void chat() {
        char account[MAXN];
        printf("Account: ");
        if (fgets(account, MAXN, stdin) == NULL) {
            printPrevious();
            return;
        }
        trimNewLine(account);
        std::string info = msgCHATREQUEST + " " + account;
        tcpWrite(fd, info);
        char buffer[MAXN];
        tcpRead(fd, buffer, MAXN);
        if (std::string(buffer).find(msgSUCCESS) != 0u) {
            printMessage(buffer, true);
            return;
        }
        char address[MAXN];
        int port;
        sscanf(buffer + msgSUCCESS.length(), "%s%d", address, &port);
        ConnectData target = connectTo(ConnectInfo(address, port));
        if (target.fd < 0) {
            printMessage("Connect Error", true);
            return;
        }
        flushScreen();
        printf("Chat to %s\n", account);
        printf("Press ^D to exit\n");
        printSplitLine();
        int checkConnCnt = 0;
        while (true) {
            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(fileno(stdin), &fdset);
            timeval tv = tv200ms;
            int nready = select(fileno(stdin) + 1, &fdset, NULL, NULL, &tv);
            if (nready < 0) {
                if (errno == EINTR) {
                    continue;
                }
                std::string errMsg = std::string("select: ") + strerror(errno);
                printMessage(errMsg);
                break;
            }
            if (FD_ISSET(fileno(stdin), &fdset)) {
                char content[MAXN];
                if (fgets(content, MAXN, stdin) == NULL) {
                    break;
                }
                trimNewLine(content);
                std::string msg = msgMESSAGE + " " + nowAccount + " " + content;
                tcpWrite(target.fd, msg);
            }
            ++checkConnCnt;
            if (checkConnCnt > 5) {
                checkConnCnt = 0;
                bool isTargetAlive = checkConnection(target.fd);
                if (!isTargetAlive) {
                    printMessage(msgDISCONNECTED, true);
                    close(target.fd);
                    return;
                }
            }
            flushMessage(account);
        }
        printMessage("Exited", true);
        close(target.fd);
    }

    void upload() {
        char account[MAXN];
        char filename[MAXN];
        printf("Account: ");
        if (fgets(account, MAXN, stdin) == NULL) {
            printPrevious();
            return;
        }
        trimNewLine(account);
        printf("Filename: ");
        if (fgets(filename, MAXN, stdin) == NULL) {
            printPrevious();
            return;
        }
        trimNewLine(filename);
        std::string filepath = std::string("./Client/") + filename;
        std::string info = msgGETUSERCONN + " " + account;
        tcpWrite(fd, info);
        char buffer[MAXN];
        tcpRead(fd, buffer, MAXN);
        if (std::string(buffer).find(msgFAIL) == 0u) {
            printMessage(buffer, true);
            return;
        }
        char address[MAXN];
        int port;
        sscanf(buffer + msgSUCCESS.length(), "%s%d", address, &port);
        struct stat fileStat;
        if (stat(filepath.c_str(), &fileStat) < 0) {
            std::string errMsg = std::string(filename) + ": " + strerror(errno);
            printMessage(errMsg, true);
            return;
        }
        FILE* fp = fopen(filepath.c_str(), "rb");
        if (!fp) {
            std::string errMsg = std::string(filename) + ": " + strerror(errno);
            printMessage(errMsg, true);
            return;
        }
        ConnectData target = connectTo(ConnectInfo(address, port));
        if (target.fd < 0) {
            printMessage("Connect Error", true);
            fclose(fp);
            return;
        }
        info = msgFILEWRITE + " " + filename + " " + std::to_string(fileStat.st_size);
        tcpWrite(target.fd, info);
        tcpRead(target.fd, buffer, MAXN);
        if (std::string(buffer).find(msgFAIL) == 0u) {
            printMessage(buffer, true);
            fclose(fp);
            close(target.fd);
            return;
        }
        printf("\nPress ^D to pause and show menu\n\n");
        unsigned long fileSize = fileStat.st_size;
        unsigned long byteSend = 0;
        bool enabled = true;
        printf("\rUploading %s... (%lu/%lu)", filename, byteSend, fileSize);
        int refreshCounter = 0;
        while (byteSend < fileSize) {
            ++refreshCounter;
            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(fileno(stdin), &fdset);
            timeval tv = tv200us;
            int nready = select(fileno(stdin) + 1, &fdset, NULL, NULL, &tv);
            if (nready < 0) {
                if (errno == EINTR) {
                    continue;
                }
                std::string errMsg = std::string("select: ") + strerror(errno);
                printMessage(errMsg);
                break;
            }
            if (FD_ISSET(fileno(stdin), &fdset)) {
                char opt[MAXN];
                if (fgets(opt, MAXN, stdin) == NULL) {
                    enabled = false;
                }
                else {
                    trimNewLine(opt);
                    toUpperString(opt);
                    if (std::string(opt) == "R") {
                        enabled = true;
                    }
                    else if (std::string(opt) == "T") {
                        break;
                    }
                }
                printf("\n\n%s...\n", enabled ? "Resumed" : "Paused");
                if (!enabled) {
                    printf("[R]Resume  [T]Terminate : ");
                }
            }
            if (enabled) {
                char content[MAXN];
                int n = fread(content, sizeof(char), MAXN, fp);
                if (tcpWritePure(target.fd, content, n) <= 0) {
                    printMessage(msgDISCONNECTED, true);
                    break;
                }
                if (tcpRead(target.fd, content, n) <= 0) {
                    printMessage(msgDISCONNECTED, true);
                    break;
                }
                byteSend += n;
                if (refreshCounter > 1000) {
                    refreshCounter = 0;
                    printf("\rUploading %s... (%lu/%lu)", filename, byteSend, fileSize);
                }
            }
        }
        fclose(fp);
        close(target.fd);
        if (byteSend == fileSize) {
            printMessage("Upload Successfully!", true);
        }
        else {
            printMessage("Terminated!", true);
        }
    }

    void download() {

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

public:
    void pushMessage(const std::string& account, const std::string& msg) {
        std::lock_guard<std::mutex> lock(clientData.msgLocker);
        clientData.msg[account].push_back(msg);
    }

private:
    void flushMessage(const std::string target) {
        if (!msgIsEmpty(target)) {
            printLog("%s%s%s: %s\n", COLOR_BRIGHT_GREEN,
                                     target.c_str(),
                                     COLOR_NORMAL,
                                     popMessage(target).c_str());
        }
    }

    bool msgIsEmpty(const std::string& account) {
        std::lock_guard<std::mutex> lock(clientData.msgLocker);
        return clientData.msg[account].empty();
    }

    std::string popMessage(const std::string& account) {
        std::lock_guard<std::mutex> lock(clientData.msgLocker);
        std::string ret = clientData.msg[account].front();
        clientData.msg[account].pop_front();
        return ret;
    }

private:
    ClientData clientData;
    std::string nowAccount;
    std::string lastmsg;
    NPStage stage;
    int fd;
    int p2pPort;
    bool needUpdateDir;

private:
    int terminalRow;
    int terminalCol;
};

#endif // NETWORK_PROGRAMMING_CLIENTUTILITY_HPP_

