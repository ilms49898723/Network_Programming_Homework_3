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
        if (nowAccount == account) {
            printMessage("Failed! To upload file to yourself is not supported", true);
            return;
        }
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
        printf("\nPress ^D to pause and show menu\n\n");
        unsigned long fileSize = fileStat.st_size;
        fileValid = true;
        fileUploadEnabled = true;
        fileSizeRead = 0u;
        ConnectInfo conn(address, port);
        std::thread uploadThread = std::thread(&ClientUtility::uploadFile, this, conn, filename, fileSize);
        printf("\rUploading %s... (%lu/%lu)", filename, fileSizeRead, fileSize);
        while (fileSizeRead < fileSize && fileValid) {
            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(fileno(stdin), &fdset);
            timeval tv = tv1s;
            int nready = select(fileno(stdin) + 1, &fdset, NULL, NULL, &tv);
            if (nready < 0) {
                if (errno == EINTR) {
                    continue;
                }
                std::string errMsg = std::string("select: ") + strerror(errno);
                printMessage(errMsg);
                fileValid = false;
                break;
            }
            if (FD_ISSET(fileno(stdin), &fdset)) {
                char option[MAXN];
                if (fgets(option, MAXN, stdin) == NULL) {
                    fileUploadEnabled = false;
                }
                else {
                    trimNewLine(option);
                    toUpperString(option);
                    if (std::string(option) == "R") {
                        fileUploadEnabled = true;
                    }
                    else if (std::string(option) == "T") {
                        fileUploadEnabled = false;
                        fileValid = false;
                        break;
                    }
                }
                printf("\n\n%s...\n", fileUploadEnabled ? "Resumed" : "Paused");
                if (!fileUploadEnabled) {
                    printf("[R]Resume  [T]Terminate:$ ");
                }
            }
            if (fileUploadEnabled) {
                printf("\rUploading %s... (%lu/%lu)", filename, fileSizeRead, fileSize);
            }
        }
        printf("\rUploading %s... (%lu/%lu)", filename, fileSizeRead, fileSize);
        uploadThread.join();
        if (fileValid) {
            printMessage("Upload Successfully!", true);
        }
        else {
            printMessage("Failed!", true);
        }
    }

    void download() {
        char option[MAXN];
        while (true) {
            printf("[D]Direct  [P]P2P:$ ");
            if (fgets(option, MAXN, stdin) == NULL) {
                printPrevious();
                return;
            }
            trimNewLine(option);
            toUpperString(option);
            if (std::string(option) == "D" || std::string(option) == "P") {
                break;
            }
        }
        if (std::string(option) == "D") {
            char account[MAXN];
            printf("Account: ");
            if (fgets(account, MAXN, stdin) == NULL) {
                printPrevious();
                return;
            }
            trimNewLine(account);
            if (nowAccount == account) {
                printMessage("Failed! To download file from yourself is not supported", true);
                return;
            }
            char filename[MAXN];
            printf("Filename: ");
            if (fgets(filename, MAXN, stdin) == NULL) {
                printPrevious();
                return;
            }
            trimNewLine(filename);
            std::string info = msgFILEINFOREQUEST + " DIRECT " + account + " " + filename;
            tcpWrite(fd, info);
            char buffer[MAXN];
            tcpRead(fd, buffer, MAXN);
            if (std::string(buffer).find(msgFAIL) == 0u) {
                printMessage(buffer, true);
                return;
            }
            char address[MAXN];
            int port;
            unsigned long fileSize;
            sscanf(buffer + msgSUCCESS.length(), "%s%d%lu", address, &port, &fileSize);
            fileValid = true;
            fileDownloadEnabled = true;
            fileSizeWritten = 0;
            ConnectInfo serverc = ConnectInfo(address, port);
            std::vector<std::thread> p2pDownloadThreads;
            p2pDownloadThreads.push_back(std::thread(&ClientUtility::downloadHandler, this, filename, fileSize));
            p2pDownloadThreads.push_back(std::thread(&ClientUtility::downloadFile, this, serverc, filename, fileSize, 0));
            for (auto& item : p2pDownloadThreads) {
                item.join();
            }
            if (fileValid) {
                updateFileList();
                printMessage("Download Successfully", true);
            }
            else {
                std::string filepath = std::string("./Client/") + filename;
                remove(filepath.c_str());
                printMessage("Failed!", true);
            }
        }
        else {
            char filename[MAXN];
            printf("Filename: ");
            if (fgets(filename, MAXN, stdin) == NULL) {
                printPrevious();
                return;
            }
            trimNewLine(filename);
            std::string info = msgFILEINFOREQUEST + " P2P " + filename;
            tcpWrite(fd, info);
            char buffer[MAXN];
            tcpRead(fd, buffer, MAXN);
            if (std::string(buffer).find(msgFAIL) == 0u) {
                printMessage(buffer, true);
                return;
            }
            std::istringstream iss(buffer);
            unsigned long fileSize;
            std::string address;
            int port;
            unsigned long from;
            unsigned long to;
            iss >> info >> fileSize;
            fileValid = true;
            fileDownloadEnabled = true;
            fileSizeWritten = 0u;
            std::vector<std::thread> p2pDownloadThreads;
            p2pDownloadThreads.push_back(std::thread(&ClientUtility::downloadHandler, this, filename, fileSize));
            while (iss >> address >> port >> from >> to) {
                ConnectInfo conn(address, port);
                p2pDownloadThreads.push_back(
                        std::thread(&ClientUtility::downloadFile, this, conn, filename, to - from, from));
            }
            for (auto& item : p2pDownloadThreads) {
                item.join();
            }
            if (fileValid) {
                updateFileList();
                printMessage("Download Successfully", true);
            }
            else {
                std::string filepath = std::string("./Client/") + filename;
                remove(filepath.c_str());
                printMessage("Failed!", true);
            }
        }
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
            printLog("%s%s%s: %s\n",
                     COLOR_BRIGHT_GREEN, target.c_str(), COLOR_NORMAL, popMessage(target).c_str());
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
    void uploadFile(const ConnectInfo& conn, const std::string& filename, unsigned long fileSize) {
        std::string filepath = std::string("./Client/") + filename;
        FILE* fp = fopen(filepath.c_str(), "rb");
        if (!fp) {
            fprintf(stderr, "%s: %s\n", filename.c_str(), strerror(errno));
            fileValid = false;
            return;
        }
        ConnectData target = connectTo(conn);
        if (target.fd < 0) {
            fileValid = false;
            fprintf(stderr, "Connect failed\n");
            fclose(fp);
            return;
        }
        std::string info = msgFILEWRITE + " " + filename + " " + std::to_string(fileSize);
        tcpWrite(target.fd, info);
        char buffer[MAXN];
        tcpRead(target.fd, buffer, MAXN);
        if (std::string(buffer).find(msgFAIL) == 0u) {
            fprintf(stderr, "%s\n", buffer);
            fileValid = false;
            fclose(fp);
            close(target.fd);
            return;
        }
        unsigned long byteSend = 0;
        while (byteSend < fileSize && fileValid) {
            if (fileUploadEnabled) {
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
                fileSizeRead += n;
            }
        }
        if (byteSend != fileSize) {
            fileValid = false;
        }
        fclose(fp);
        close(target.fd);
    }

private:
    void downloadHandler(const std::string& filename, unsigned long fileSize) {
        printf("\nPress ^D to pause and show menu\n\n");
        printf("\rDownloading %s... (%lu/%lu)", filename.c_str(), fileSizeWritten, fileSize);
        while (fileSizeWritten < fileSize) {
            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(fileno(stdin), &fdset);
            timeval tv = tv1s;
            int nready = select(fileno(stdin) + 1, &fdset, NULL, NULL, &tv);
            if (nready < 0) {
                if (errno == EINTR) {
                    continue;
                }
                std::string errMsg = std::string("select: ") + strerror(errno);
                printMessage(errMsg);
                fileValid = false;
                break;
            }
            if (FD_ISSET(fileno(stdin), &fdset)) {
                char option[MAXN];
                if (fgets(option, MAXN, stdin) == NULL) {
                    setFileDownloadEnabled(false);
                }
                else {
                    trimNewLine(option);
                    toUpperString(option);
                    if (std::string(option) == "R") {
                        setFileDownloadEnabled(true);
                    }
                    else if (std::string(option) == "T") {
                        fileValid = false;
                        break;
                    }
                }
                printf("\n\n%s...\n", fileDownloadEnabled ? "Resumed" : "Paused");
                if (!fileDownloadEnabled) {
                    printf("[R]Resume  [T]Terminate:$ ");
                }
            }
            if (fileDownloadEnabled) {
                printf("\rDownloading %s... (%lu/%lu)", filename.c_str(), fileSizeWritten, fileSize);
            }
        }
        printf("\rDownloading %s... (%lu/%lu)", filename.c_str(), fileSizeWritten, fileSize);
        if (fileSizeWritten != fileSize) {
            fileValid = false;
        }
    }

    void downloadFile(const ConnectInfo& conn, const std::string& filename, unsigned long fileSize, unsigned long offset) {
        ConnectData target = connectTo(conn);
        if (target.fd < 0) {
            printMessage("Connect failed", true);
            fileValid = false;
            return;
        }
        std::string info = msgFILEREAD + " CONFIRM " + filename + " " +
                           std::to_string(offset) + " " + std::to_string(fileSize);
        tcpWrite(target.fd, info);
        char buffer[MAXN];
        tcpRead(target.fd, buffer, MAXN);
        if (std::string(buffer).find(msgFAIL) == 0u) {
            printMessage(buffer, true);
            close(target.fd);
            fileValid = false;
            return;
        }
        unsigned long byteRecv = 0;
        char* fileCache = new char[MAXN * 1024];
        unsigned cacheIndex = 0u;
        unsigned cacheOffset = 0u;
        unsigned cacheSize = 0u;
        while (byteRecv < fileSize && fileValid) {
            if (fileDownloadEnabled) {
                if (cacheSize > MAXN * 1000) {
                    if (fileWrite(filename, offset + cacheIndex, fileCache, cacheSize) <= 0) {
                        fileValid = false;
                        break;
                    }
                    cacheIndex += cacheSize;
                    cacheOffset = 0u;
                    cacheSize = 0u;
                }
                int n;
                if ((n = tcpRead(target.fd, fileCache + cacheOffset, MAXN)) <= 0) {
                    fileValid = false;
                    break;
                }
                if (tcpWrite(target.fd, msgSUCCESS) <= 0) {
                    fileValid = false;
                    break;
                }
                byteRecv += n;
                cacheOffset += n;
                cacheSize += n;
                addFileSizeWritten(n);
            }
        }
        if (cacheSize > 0u) {
            if (fileWrite(filename, offset + cacheIndex, fileCache, cacheSize) <= 0) {
                fileValid = false;
            }
        }
        if (byteRecv != fileSize) {
            fileValid = false;
        }
        delete[] fileCache;
        close(target.fd);
    }

    int fileWrite(const std::string& filename, unsigned long offset, const char* src, int n) {
        std::lock_guard<std::mutex> lock(fileWriteLocker);
        std::string filepath = std::string("./Client/") + filename;
        FILE* fp = fopen(filepath.c_str(), "wb");
        if (!fp) {
            fprintf(stderr, "%s: %s\n", filename.c_str(), strerror(errno));
            fileValid = false;
            return -1;
        }
        fseek(fp, offset, SEEK_SET);
        int ret = fwrite(src, sizeof(char), n, fp);
        fclose(fp);
        return ret;
    }

    void addFileSizeWritten(const unsigned long val) {
        std::lock_guard<std::mutex> lock(fileSizeWrittenLocker);
        fileSizeWritten += val;
    }

    void setFileDownloadEnabled(const bool enabled) {
        std::lock_guard<std::mutex> lock(fileWriteLocker);
        fileDownloadEnabled = enabled;
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

private:
    bool fileValid;

private:
    unsigned long fileSizeRead;
    bool fileUploadEnabled;

private:
    std::mutex fileWriteLocker;
    std::mutex fileSizeWrittenLocker;
    unsigned long fileSizeWritten;
    bool fileDownloadEnabled;
};

#endif // NETWORK_PROGRAMMING_CLIENTUTILITY_HPP_

