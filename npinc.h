#ifndef NETWORK_PROGRAMMING_HOMEWORK_2_NPINC_H_
#define NETWORK_PROGRAMMING_HOMEWORK_2_NPINC_H_

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

constexpr int MAXN = 2048;

enum class NPStage {
    INIT, WELCOME, MAIN, ARTICLE, SEARCH, FRIENDS, CHAT, CHATGROUP
};

enum class NPArticlePermission {
    PUBLIC, AUTHOR, FRIENDS, SPEC
};

#endif // NETWORK_PROGRAMMING_HOMEWORK_2_NPINC_H_

