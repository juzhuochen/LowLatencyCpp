#pragma once
#include <iostream>
#include <string>
#include <unordered_set>
#include <sstream>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <fcntl.h>

#include "macros.h"
#include "logging.h"

namespace Common {
    
constexpr int MaxTCPServerBackLog = 1024;
auto getIfaceIP(const std::string &iface) -> std::string;
auto setNonBlocking(int fd) -> bool;
auto setNoDelay(int fd) -> bool;
auto setSOtimestamp(int fd) -> bool;
auto wouldBlock() -> bool;
auto setMcastTTL(int fd, int mcast_ttl) -> bool;
auto setTTL(int fd,int ttl)
    -> bool;
auto join(int fd, const std::string &ip, const std::string &iface, int port)
    -> bool;
auto creatSocket(
    Logger &logger,
    const std::string &t_ip,
    const std::string &iface,
    int port,
    bool is_udp,
    bool is_blocking,
    bool is_listening,
    int ttl,
    bool need_to_timestamp) -> int;

} // namespace Common