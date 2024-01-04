#pragma once
#include "time_utils.h"
#include "logging.h"
#include "macros.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <ifaddrs.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace Common {
struct SocketCfg {
    std::string m_ip;
    std::string m_iface;
    int m_port = -1;
    bool m_is_udp = false;
    bool m_is_listening = false;
    bool m_need_so_timestamp = false;
    [[nodiscard]] auto toString() const {
        std::stringstream ss;
        ss << "SocketCfd[ip: " << m_ip << " iface: " << m_iface
           << " port: " << m_port << " is_udp: " << m_is_udp
           << " is_listening: " << m_is_listening
           << " needs_SO_timestamp: " << m_need_so_timestamp << "]";
        return ss.str();
    }
};

constexpr int MaxTCPServerBackLog = 1024;

inline auto getIfaceIP(const std::string &iface) -> std::string {
    char buf[NI_MAXHOST] = {'\0'};
    ifaddrs *ifaddr = nullptr;
    if (getifaddrs(&ifaddr) != -1) {
        for (ifaddrs *ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET
                && iface == ifa->ifa_name) {
                getnameinfo(
                    ifa->ifa_addr, sizeof(sockaddr_in), buf, sizeof(buf),
                    nullptr, 0, NI_NUMERICHOST);
                break;
            }
        }
        freeifaddrs(ifaddr);
    }
    return buf;
}
inline auto setNonBlocking(int fd) -> bool {
    const auto flags = fcntl(fd, F_GETFL, 0);
    if (flags & O_NONBLOCK) { return true; }
    return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1);
}

inline auto disableNagle(int fd) -> bool {
    int one = 1;
    return (
        setsockopt(
            fd, IPPROTO_TCP, TCP_TIMESTAMP, reinterpret_cast<void *>(&one),
            sizeof(one))
        != -1);
}
inline auto setSOtimestamp(int fd) -> bool {
    int one = 1;
    return (
        setsockopt(
            fd, SOL_SOCKET, TCP_TIMESTAMP, reinterpret_cast<void *>(&one),
            sizeof(one))
        != -1);
}
// auto wouldBlock() -> bool;
// auto setMcastTTL(int fd, int mcast_ttl) -> bool;
// auto setTTL(int fd, int ttl) -> bool;
inline auto join(int fd, const std::string &ip) -> bool {
    const ip_mreq mreq{{inet_addr(ip.c_str())}, {htonl(INADDR_ANY)}};
    return (
        setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))
        != -1);
}
inline auto creatSocket(Logger &logger, const SocketCfg &socket_cfg) -> int {
    std::string time_str;
    const auto ip = socket_cfg.m_ip.empty() ? getIfaceIP(socket_cfg.m_iface)
                                            : socket_cfg.m_ip;
    logger.log(
        "%:% %() % cfg:%\n", __FILE__, __LINE__, __FUNCTION__,
        Common::getCurrentTimeStr(&time_str), socket_cfg.toString());

    const int input_flags = (socket_cfg.m_is_listening ? AI_PASSIVE : 0)
                            | (AI_NUMERICHOST | AI_NUMERICSERV);
    const addrinfo hints{
        input_flags,
        AF_INET,
        socket_cfg.m_is_udp ? SOCK_DGRAM : SOCK_STREAM,
        socket_cfg.m_is_udp ? IPPROTO_UDP : IPPROTO_TCP,
        0,
        0,
        nullptr,
        nullptr};
    addrinfo *result = nullptr;
    const auto rc = getaddrinfo(
        ip.c_str(), std::to_string(socket_cfg.m_port).c_str(), &hints, &result);
    ASSERT(
        !rc, "getaddrinfo() failed. error:" + std::string(gai_strerror(rc))
                 + " errno: " + strerror(errno));
    int socket_fd = -1;
    int one = 1;
    for (addrinfo *rp = result; rp; rp = rp->ai_next) {
        ASSERT(
            socket_fd =
                socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol) != -1,
            "socket() failed. errno:" + std::string(strerror(errno)));
        ASSERT(
            setNonBlocking(socket_fd),
            "setNonblocking() failed. errno: " + std::string(strerror(errno)));
        if (!socket_cfg.m_is_udp) { // disable nagle for tcp
            ASSERT(
                disableNagle(socket_fd), "disableNagle() failed. errno: "
                                             + std::string(strerror(errno)));
        }
        if (!socket_cfg.m_is_listening) {
            ASSERT(
                connect(socket_fd, rp->ai_addr, rp->ai_addrlen) != -1,
                "connect() failed. errno: " + std::string(strerror(errno)));
        }
        if (!socket_cfg.m_is_listening) {
            ASSERT(
                setsockopt(
                    socket_fd, SOL_SOCKET, SO_REUSEADDR,
                    reinterpret_cast<const char *>(&one), sizeof(one)),
                "setsockopt() SO_REUSEADDR failed. errno:"
                    + std::string(strerror(errno)));
        }
        if (socket_cfg.m_is_listening) {
            const sockaddr_in addr{
                AF_INET, htons(socket_cfg.m_port), {htonl(INADDR_ANY)}, {}};
            ASSERT(
                bind(
                    socket_fd,
                    socket_cfg.m_is_udp
                        ? reinterpret_cast<const struct sockaddr *>(&addr)
                        : rp->ai_addr,
                    sizeof(addr))
                    == 0,
                "bind() failed. errno:%" + std::string(strerror(errno)));
        }
        if (!socket_cfg.m_is_udp
            && socket_cfg.m_is_listening) { // listen for tcp
            ASSERT(
                listen(socket_fd, MaxTCPServerBackLog) == 0,
                "listen() failed. errno:" + std::string(strerror(errno)));
        }
        if (socket_cfg.m_need_so_timestamp) {
            ASSERT(
                setSOtimestamp(socket_fd), "setSOtimestamp() failed. errno:"
                                               + std::string(strerror(errno)));
        }
    }
    if (result) { freeaddrinfo(result); }
    return socket_fd;
}

} // namespace Common