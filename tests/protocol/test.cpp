//
// Created by andre on 29/10/2026.
//

#include "gtest/gtest.h"
#include "protocol/protocol.h"

// TEST(Connection, EncodesMessageLengthInNetworkByteOrder)
// {
//     fd_t fd[2];
//     auto i = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
//
//     const auto size = 3;
//     const auto sending = htonl(size);
//     transport::protocol::send_all(fd[0], &sending, sizeof(sending));
//
//     std::uint32_t received{};
//     transport::protocol::recv_all(fd[1], &received, sizeof(received));
//
//     ASSERT_EQ(sending, received);
//     ASSERT_EQ(size, ntohl(received));
// }
