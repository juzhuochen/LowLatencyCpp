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
struct SocketCfg {
    std::string m_ip;
    std::string m_iface;
    int m_port = -1;
    bool m_is_udp = false;
    bool m_is_listening = false;
    bool m_need_so_timestamp = false;
    [[nodiscard]]auto toString() const {
        std::stringstream ss;
        ss << "SocketCfd[ip: " << m_ip << " iface: " << m_iface
           << " port: " << m_port << " is_udp: " << m_is_udp
           << " is_listening: " << m_is_listening
           << " needs_SO_timestamp: " << m_need_so_timestamp << "]";
        return ss.str();
    }
};

constexpr int MaxTCPServerBackLog = 1024;
auto getIfaceIP(const std::string &iface) -> std::string{
    char buf[NI_MAXHOST]={'\0'};
    ifaddrs* ifaddr=nullptr;
    if(getifaddrs(&ifaddr)!=-1){
        for (ifaddr* ifa=ifaddr; ifaddr; ifa=ifa->ifa_next) {
        if(){

        }
        }
    }
}
auto setNonBlocking(int fd) -> bool;
auto setNoDelay(int fd) -> bool;
auto setSOtimestamp(int fd) -> bool;
auto wouldBlock() -> bool;
auto setMcastTTL(int fd, int mcast_ttl) -> bool;
auto setTTL(int fd, int ttl) -> bool;
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