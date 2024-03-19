// #include "tcp_socket.h"
#include "time_utils.h"
#include "logging.h"
#include "tcp_server.h"
int main(int cmd, char **) {
    using namespace Common;
    std::string time_str;
    Logger logger("socket_example.log");
    auto tcpServerRecvCallback = [&](TCPSocket *socket,
                                     Nanos rx_time) noexcept {
        logger.log(
            "TCPServer::defaultRecvCallback() socket:% len:% rx:%\n",
            socket->m_socket_fd, socket->m_next_rcv_valid_idx, rx_time);
    };
    const std::string response = "TCPserver received msg: " + std::string();
    return 0;
}