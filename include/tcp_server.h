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
    int m_epoll_fd = -1;
    TCPSocket m_listener_socket;
    epoll_event m_evens[1024];
    // cokkection of all sockert, socket for incoming and outgoing data
    std::vector<TCPSocket *> m_recv_sockets, m_send_socket;
    // function warpper to call back when data is available.
    std::function<void(TCPSocket *s, Nanos rx_time)> m_recv_callback = nullptr;
    // function warpper to call back when all data across all TcpSockets
    // has been read and dispatched this round
    std::function<void()> m_recv_finished_callback = nullptr;
    std::string m_time_str;
    Logger &m_logger;
};
} // namespace Common
