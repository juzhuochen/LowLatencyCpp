#pragma once
#include "socket_utils.h"
#include "logging.h"
#include <cstddef>
#include <functional>
#include <netinet/in.h>
#include <vector>

namespace Common {
constexpr size_t TCPBufferSize = (size_t)64 * 1024 * 1024;
struct TCPSocket {
    explicit TCPSocket(Logger &logger) : m_logger(logger) {
        m_outbound_data.resize(TCPBufferSize);
        m_inbound_data.resize(TCPBufferSize);
    }
    auto connect(
        const std::string &ip,
        const std::string &iface,
        int port,
        bool is_listening) -> int;
    auto sendAndRecv() noexcept -> bool;
    auto send(const void* data,size_t len) noexcept -> void;

    TCPSocket() = delete;
    TCPSocket(const TCPSocket &) = delete;
    TCPSocket(const TCPSocket &&) = delete;
    TCPSocket &operator=(const TCPSocket &) = delete;
    TCPSocket &operator=(const TCPSocket &&) = delete;
    int m_socket_fd = -1;
    std::vector<char> m_outbound_data;
    size_t m_next_send_valid_idx = 0;
    std::vector<char> m_inbound_data;
    size_t m_next_rcv_valid_idx = 0;
    struct sockaddr_in m_socket_attrib {};

    std::function<void(TCPSocket *s, Nanos rx_time)> m_recv_callback = nullptr;
    std::string m_time_str;
    Logger &m_logger;
};
} // namespace Common