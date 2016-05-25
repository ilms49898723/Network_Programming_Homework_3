#ifndef NETWORK_PROGRAMMING_THREADUTIL_HPP_
#define NETWORK_PROGRAMMING_THREADUTIL_HPP_

#include <cstdio>
#include <sstream>
#include <chrono>
#include <vector>
#include <utility>
#include <thread>
#include <mutex>
#include "nputility.hpp"

namespace lb {

// thread related variables
static bool valid;
std::mutex threadLocker;
// vector of threads
std::vector<std::pair<std::thread, bool>> threads;
// valid getter, setter
void setValid(const bool val);
bool isValid();
// thread related functions
void threadManageInit();
void threadMaintain();
void finishThread();
void joinAll(bool logEnabled = true);
std::string getThreadIdStr(const std::thread::id id);

void setValid(const bool val) {
    valid = val;
}

bool isValid() {
    return valid;
}

void threadManageInit() {
    valid = true;
    threadLocker.lock();
    threads.push_back(std::make_pair(std::thread(threadMaintain), true));
    threadLocker.unlock();
}

void pushThread(std::thread&& item) {
    threadLocker.lock();
    threads.push_back(std::make_pair(std::move(item), true));
    threadLocker.unlock();
}

void threadMaintain() {
    while (valid) {
        threadLocker.lock();
        bool flag = true;
        while (flag) {
            flag = false;
            unsigned toRemove = 0xFFFFFFFFu;
            for (unsigned i = 0; i < threads.size(); ++i) {
                if (!threads.at(i).second) {
                    flag = true;
                    toRemove = i;
                }
            }
            if (flag) {
                threads.at(toRemove).first.join();
                threads.erase(threads.begin() + toRemove);
            }
        }
        threadLocker.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void finishThread() {
    std::lock_guard<std::mutex> lock(threadLocker);
    for (auto& item : threads) {
        if (std::this_thread::get_id() == item.first.get_id()) {
            item.second = false;
            break;
        }
    }
}

void joinAll(bool logEnabled) {
    valid = false;
    for (auto& item : threads) {
        if (item.first.joinable()) {
            item.first.join();
            if (logEnabled) {
                printLog("Thread id %s finished\n", getThreadIdStr(item.first.get_id()).c_str());
            }
        }
    }
}

std::string getThreadIdStr(const std::thread::id id) {
    std::ostringstream oss;
    oss << std::hex << id;
    return oss.str();
}

} // namespace lb

#endif // NETWORK_PROGRAMMING_THREADUTIL_HPP_

