#include "tcp_server.h"
#include "macros.h"
#include "socket_utils.h"
#include "tcp_socket.h"
#include <algorithm>
#include <cstring>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace Common {
auto TCPServer::addToEpollList(TCPSocket *socket) {
    epoll_event ev{EPOLLET | EPOLLIN, {reinterpret_cast<void *>(socket)}};
    return !epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, socket->m_socket_fd, &ev);
}
auto TCPServer::listen(const std::string &iface, int port) -> void {
    m_epoll_fd = epoll_create(1);
    ASSERT(
        m_epoll_fd >= 0,
        "epoll_create() failed. errno:" + std::string(std::strerror(errno)));
    ASSERT(
        m_listener_socket.connect("", iface, port, true) >= 0,
        "listener socker failed to connect. iface: " + iface + " port"
            + std::to_string(port)
            + " errno: " + std::string(std::strerror(errno)));
    ASSERT(
        addToEpollList(&m_listener_socket),
        "epoll_ctl() failed. errno: " + std::string(std::strerror(errno)));
}
auto TCPServer::sendAndRecv() noexcept -> void {
    auto recv = false;
    std::for_each(
        m_recv_sockets.begin(), m_recv_sockets.end(),
        [&recv](auto socket) { recv |= socket->sendAndRecv(); });
}
auto TCPServer::poll() noexcept -> void {
    const int max_events = 1 + m_send_socket.size() + m_recv_sockets.size();
    const int n = epoll_wait(m_epoll_fd, m_evens, max_events, 0);
    bool have_new_connection = false;
    for (int i = 0; i < n; ++i) {
        const auto &event = m_evens[i];
        auto socket = reinterpret_cast<TCPSocket *>(event.data.ptr);

        //
        if (event.events & EPOLLIN) {
            if (socket == &m_listener_socket) {
                m_logger.log(
                    "%: %() % EPOLLIN m_listen_socket:%\n", __FILE__, __LINE__,
                    __FUNCTION__, Common::getCurrentTimeStr(&m_time_str),
                    socket->m_socket_fd);
                have_new_connection = true;
                continue;
            }

            m_logger.log(
                "%:% %() % EPOLLIN socket:%\n", __FILE__, __LINE__,
                __FUNCTION__, Common::getCurrentTimeStr(&m_time_str),
                socket->m_socket_fd);
            if (std::find(m_recv_sockets.begin(), m_recv_sockets.end(), socket)
                == m_recv_sockets.end()) {
                m_recv_sockets.push_back(socket);
            }
        }
        //
        if (event.events & EPOLLOUT) {
            m_logger.log(
                "%:% %() % EPOLLOUT socket:%\n", __FILE__, __LINE__,
                __FUNCTION__, Common::getCurrentTimeStr(&m_time_str),
                socket->m_socket_fd);
            if (std::find(m_send_socket.begin(), m_send_socket.end(), socket)
                == m_send_socket.end()) {
                m_send_socket.push_back(socket);
            }
        }
        //
        if (event.events & (EPOLLERR | EPOLLHUP)) {
            m_logger.log(
                "%:% %() % EPOLLERR socket:%\n", __FILE__, __LINE__,
                __FUNCTION__, Common::getCurrentTimeStr(&m_time_str),
                socket->m_socket_fd);
            if (std::find(m_recv_sockets.begin(), m_recv_sockets.end(), socket)
                == m_recv_sockets.end()) {
                m_recv_sockets.push_back(socket);
            }
        }
        //
        while (have_new_connection) {
            m_logger.log(
                "%:% %() % have_new_connection\n", __FILE__, __LINE__,
                __FUNCTION__, Common::getCurrentTimeStr(&m_time_str));
            sockaddr_storage addr;
            socklen_t addr_len = sizeof(addr);
            int fd = accept(
                m_listener_socket.m_socket_fd,
                reinterpret_cast<sockaddr *>(&addr), &addr_len);
            if (fd == -1) { break; }
            ASSERT(
                setNonBlocking(fd) && disableNagle(fd),
                "Failed to set non-blocking or no-delay on socket:"
                    + std::to_string(fd));
            m_logger.log(
                "%:% %() % accepted socket:%\n", __FILE__, __LINE__,
                __FUNCTION__, Common::getCurrentTimeStr(&m_time_str), fd);

            auto socket = new TCPSocket(m_logger);
            socket->m_socket_fd = fd;
            socket->m_recv_callback = m_recv_callback;
            ASSERT(
                addToEpollList(socket),
                "Unable to add socket. error:"
                    + std::string(std::strerror(errno)));

            if (std::find(m_recv_sockets.begin(), m_recv_sockets.end(), socket)
                == m_recv_sockets.end()) {
                m_recv_sockets.push_back(socket);
            }
        }
    }
}
} // namespace Common