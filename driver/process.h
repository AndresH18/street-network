//
// Created by andre on 30/10/2026.
//

#ifndef ECE650_A4_PROCESS_H
#define ECE650_A4_PROCESS_H

#include <unistd.h>
#include <utility>

#include "protocol/command.h"
#include "protocol/connection.h"

namespace driver {
    struct ProcessConfig {
        std::string name;
        std::string executable;
        std::vector<std::string> args;
    };

    /**
     * @class Process
     * @brief Represents a process.
     *
     * Provides an abstraction for interacting another process. This class encapsulates
     * the process lifecycle and status.
     *
     */
    class Process {
        pid_t pid_;
        int exit_status_{};
        bool finished_{false};
        unsigned int request_count_{};
        transport::Connection connection_;

      public:
        Process(pid_t pid, transport::Connection connection);
        ~Process();

        Process(const Process&) = delete;
        Process& operator=(const Process&) = delete;

        Process(Process&& other) noexcept
            : pid_{std::exchange(other.pid_, -1)},
              exit_status_{other.exit_status_},
              finished_{other.finished_},
              request_count_{other.request_count_},
              connection_{std::move(other.connection_)}
        {
            // terminate();
            other.finished_ = true;
        }

        Process& operator=(Process&& other) noexcept
        {
            if (this == &other)
                return *this;
            // terminate();
            pid_ = other.pid_;
            exit_status_ = other.exit_status_;
            finished_ = other.finished_;
            connection_ = std::move(other.connection_);
            request_count_ = other.request_count_;
            return *this;
        }

        friend Process make_process(ProcessConfig config);

        [[nodiscard]] bool send(const command::Command& command) const;
        bool request_shutdown() const;
        bool terminate() const;
        int wait();

        [[nodiscard]] int exit_code() const
        {
            return exit_status_;
        }

        [[nodiscard]] pid_t pid() const
        {
            return pid_;
        }

        [[nodiscard]] const transport::Connection& connection() const
        {
            return connection_;
        }
    };

    Process make_process(ProcessConfig config);


} // namespace driver

#endif // ECE650_A4_PROCESS_H
