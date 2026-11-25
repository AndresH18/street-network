//
// Created by andres on 10/27/26.
//

#ifndef ECE650_A4_PROTOCOL_H
#define ECE650_A4_PROTOCOL_H


#include <cstring>
#include <span>
#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "log.h"

typedef int fd_t;

namespace transport::protocol {
    /**
     * Maximum allowed message size (64 KiB).
     */
    constexpr std::uint32_t MAX_MESSAGE_SIZE = 64 * 1024; // 64 KiB

    /**
     * Sends a single packet through a SOCK_SEQPACKET socket.
     *
     * @param fd     Socket file descriptor.
     * @param buffer Message data to send.
     *
     * @return true if the entire packet was sent; false otherwise.
     *
     * @note Empty messages and messages exceeding MAX_MESSAGE_SIZE are rejected.
     * @note Retries automatically if interrupted by EINTR.
     */
    inline bool send(const fd_t fd, const std::span<const std::byte> buffer)
    {
        if (buffer.empty() || buffer.size() > MAX_MESSAGE_SIZE) {
            return false;
        }

        while (true) {
            const ssize_t sent = ::send(fd, buffer.data(), buffer.size(), MSG_NOSIGNAL);
            if (sent == static_cast<ssize_t>(buffer.size()))
                return true;

            if (sent >= 0) { // partial send
                LOG_TRACE("sendmsg sent partial message");
                return false;
            }

            if (errno == EINTR) {
                LOG_TRACE("send interrupted by EINTR");
                continue;
            }

            LOG_TRACE("sendmsg failed with error: {}", strerror(errno));

            return false;
        }
    }

    /**
     * Receives a single packet from a SOCK_SEQPACKET socket.
     *
     * @param fd     Socket file descriptor.
     * @param buffer Receives the message data.
     *
     * @return true if a complete packet was received; false otherwise.
     *
     * @note Packets exceeding MAX_MESSAGE_SIZE are rejected.
     * @note Retries automatically if interrupted by EINTR.
     * @note The buffer is resized to the actual received message size.
     */
    inline bool recv(const fd_t fd, std::vector<std::byte>& buffer)
    {
        buffer.resize(MAX_MESSAGE_SIZE);
        while (true) {
            iovec iov{
                    .iov_base = buffer.data(),
                    .iov_len = buffer.size(),
            };

            msghdr msg{
                    .msg_iov = &iov,
                    .msg_iovlen = 1,
            };

            if (const ssize_t received = recvmsg(fd, &msg, 0); received >= 0) {
                if (received == 0) // connection closed, EOF
                {
                    LOG_TRACE("Connection closed, EOF");
                    return false;
                }

                if (msg.msg_flags & MSG_TRUNC) {
                    LOG_TRACE("recvmsg truncated message");
                    buffer.clear();
                    return false;
                }

                buffer.resize(static_cast<size_t>(received));
                return true;
            }

            if (errno == EINTR) {
                LOG_TRACE("recvmsg interrupted by EINTR");
                continue;
            }

            LOG_TRACE("recvmsg failed with error: {}", strerror(errno));

            return false;
        }
    }

} // namespace transport::protocol
#endif // ECE650_A4_PROTOCOL_H
