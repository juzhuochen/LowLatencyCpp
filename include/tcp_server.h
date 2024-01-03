#pragma once
#include "logging.h"
#include "tcp_socket.h"
#include <functional>
#include <sys/epoll.h>
#include <vector>
namespace Common {
struct TCPServer {
    explicit TCPServer(Logger &logger)
        : m_listener_socket(logger), m_logger(logger) {}
    auto listen(const std::string &iface, int port) -> void;
    auto poll() noexcept -> void;
    auto sendAndRecv() noexcept -> void;

  private:
    auto addToEpollList(TCPSocket *socket);

  public:
    int epoll_fd = -1;
    TCPSocket m_listener_socket;
    epoll_event m_evens[1024];
    std::vector<TCPSocket *> m_recv_sockets, m_send_socket;
    std::function<void(TCPSocket *s, Nanos rx_time)> m_recv_callback = nullptr;
    std::function<void()> recv_finished_callback = nullptr;
    std::string m_time_str;
    Logger &m_logger;
};
} // namespace Common
