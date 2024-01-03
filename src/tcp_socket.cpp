#include "tcp_socket.h"
#include "socket_utils.h"
#include <asm-generic/socket.h>
#include <bits/types/struct_iovec.h>
#include <bits/types/struct_timeval.h>
#include <cstddef>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <system_error>
namespace Common {
auto TCPSocket::connect(
    const std::string &ip,
    const std ::string &iface,
    int port,
    bool is_listening) -> int {
    const SocketCfg socket_cfg = {ip, iface, port, false, is_listening, true};

    m_socket_fd = creatSocket(m_logger, socket_cfg);
    m_socket_attrib.sin_addr.s_addr = INADDR_ANY;
    m_socket_attrib.sin_port = htons(port);
    m_socket_attrib.sin_family = AF_INET;
    return m_socket_fd;
}

auto TCPSocket::sendAndRecv() noexcept -> bool {
    char ctrl[CMSG_SPACE(sizeof(struct timeval))];
    auto *cmsg = reinterpret_cast<struct cmsghdr *>(&ctrl);
    iovec iov{
        m_inbound_data.data() + m_next_rcv_valid_idx,
        TCPBufferSize - m_next_rcv_valid_idx};
    msghdr msg{&m_socket_attrib,
               sizeof(m_socket_attrib),
               &iov,
               1,
               ctrl,
               sizeof(ctrl),
               0};
    const auto read_size = recvmsg(m_socket_fd, &msg, MSG_DONTWAIT);
    if (read_size > 0) {
        m_next_rcv_valid_idx += read_size;
        Nanos kernel_time = 0;
        timeval time_kernel;
        if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_TIMESTAMP
            && cmsg->cmsg_len == CMSG_LEN(sizeof(time_kernel))) {
            memcpy(&time_kernel, CMSG_DATA(cmsg), sizeof(time_kernel));
            kernel_time =
                time_kernel.tv_sec * NANOS_TO_SEC
                + time_kernel.tv_usec
                      * NANOS_TO_MICROS; // convert timestamp to nanoseconds
        }
        const auto user_time = getCurrentNaos();
        m_logger.log(
            "%:% %() % read socket:% len:% utime:% ktime:% diff:%\n", __FILE__,
            __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&m_time_str),
            m_socket_fd, m_next_rcv_valid_idx, user_time, kernel_time,
            (user_time - kernel_time));
        m_recv_callback(this, kernel_time);
    }
    if (m_next_send_valid_idx > 0) {
        // non-blocking call to send data
        const auto n = ::send(
            m_socket_fd, m_outbound_data.data(), m_next_send_valid_idx,
            MSG_DONTWAIT | MSG_NOSIGNAL);
        m_logger.log(
            "%:% %() send socket:% len:%\n", __FILE__, __LINE__, __FUNCTION__,
            Common::getCurrentTimeStr(&m_time_str), m_socket_fd, n);
    }
    m_next_send_valid_idx = 0;

    return (read_size > 0);
}

auto TCPSocket::send(const void *data, size_t len) noexcept -> void {
    memcpy(m_outbound_data.data() + m_next_send_valid_idx, data, len);
    m_next_send_valid_idx += len;
}
} // namespace Common