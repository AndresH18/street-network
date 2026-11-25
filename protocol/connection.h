//
// Created by andres on 10/28/26.
//

#ifndef ECE650_A4_CONNECTION_HPP
#define ECE650_A4_CONNECTION_HPP

#include <getopt.h>
#include <stdexcept>
#include <utility>

#include <sys/socket.h>

#include "command.h"
#include "log.h"

typedef int fd_t;

namespace transport {
    /**
     * @class Connection
     * @brief Represents a managed connection to an external resource or service.
     *
     * The Connection class encapsulates the lifecycle and behavior required to
     * establish, maintain, validate, and close a connection. It provides a common
     * abstraction for working with a communication channel while hiding underlying
     * connection details from consumers.
     *
     * This class is intended to centralize connection-related responsibilities such
     * as initialization, state management, error handling, and cleanup. Implementations
     * may use it to interact with databases, remote APIs, network services, message
     * brokers, or other external systems that require an explicit connection phase.
     *
     * @note The exact behavior of the connection depends on the concrete
     * implementation and the resource being accessed.
     */
    class Connection {
        fd_t fd_;

        void close();

      public:
        explicit Connection(const fd_t fd)
            : fd_{fd}
        {
            if (fd < 0) {
                LOG_CRIT("Invalid fd {}", fd);
                throw std::runtime_error("Invalid fd");
            }
        }

        ~Connection()
        {
            close();
        }

        Connection(const Connection& other) = delete;

        Connection(Connection&& other) noexcept
            : fd_{std::exchange(other.fd_, -1)}
        {
        }

        Connection& operator=(const Connection& other) = delete;

        Connection& operator=(Connection&& other) noexcept
        {
            if (this == &other)
                return *this;

            close();

            fd_ = other.fd_;
            other.fd_ = -1;
            return *this;
        }

        [[nodiscard]] fd_t fd() const
        {
            return fd_;
        }

        /**
         * @brief Sends the @p command to the underlying connection.
         *
         * @param command The command to be sent.
         * @return A value indicating the result of the send operation.
         */
        [[nodiscard]] bool send(const command::Command& command) const;

        /**
         * @brief Receives a command from the underlying connection and puts it on the @p command
         *
         * @return @c true  on success, @c false otherwise
         */
        bool recv(command::Command& command) const;
    };

    /**
     * @brief Creates a pair of connected local transport connections.
     *
     * Creates a UNIX domain socket pair using sequenced-packet semantics and wraps
     * each file descriptor in a Connection instance. The returned connections are
     * connected to each other, allowing data sent through one endpoint to be
     * received from the other.
     *
     * @return A pair containing the two connected Connection endpoints.
     */
    inline std::pair<Connection, Connection> create_connection_pair()
    {
        static constexpr int PROTOCOL = 0;

        fd_t fd[2];
        ::socketpair(PF_UNIX, SOCK_SEQPACKET, PROTOCOL, fd);

        Connection first{fd[0]};
        Connection second{fd[1]};

        return {std::move(first), std::move(second)};
    }

    /**
     * @brief Parses the socket file descriptor from command-line arguments.
     *
     * Reads the required socket descriptor from the `--socket` or `-s` command-line
     * option, converts it to an integer, and returns it when it represents a
     * non-negative file descriptor value.
     *
     * @param argc The number of command-line arguments.
     * @param argv The command-line argument vector to parse.
     * @return The non-negative socket file descriptor specified by the socket option.
     *
     * @throws std::runtime_error If an unknown option is encountered or the parsed
     * socket value is negative.
     * @throws std::invalid_argument If the socket option value cannot be converted
     * to an integer.
     * @throws std::out_of_range If the socket option value is outside the range of
     * representable integer values.
     */
    inline int get_socket(const int argc, char* argv[])
    {
        std::string socket;

        static constexpr option arg_options[] = {
                {.name = "socket", .has_arg = required_argument, .flag = nullptr, .val = 's'},
                {.name = nullptr, .has_arg = 0, .flag = nullptr, .val = 0}};

        int opt;
        while ((opt = getopt_long(argc, argv, "s:", arg_options, nullptr)) != -1) {
            if (opt == 's') {
                socket = optarg;
            } else if (opt == '?') {
                throw std::runtime_error("Unknown: " + std::string{static_cast<char>(optopt)});
            }
        }

        if (const int i = std::stoi(socket); i >= 0)
            return i;

        LOG_CRIT("Invalid socket {}", socket);

        throw std::runtime_error("Invalid socket");
    }
} // namespace transport

#endif // ECE650_A4_CONNECTION_HPP
