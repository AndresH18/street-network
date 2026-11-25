//
// Created by andre on 30/10/2026.
//

#include "process.h"

#include <csignal>
#include <cstdio>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

#include <sys/wait.h>

#include "protocol/log.h"

namespace driver {

    Process::Process(const pid_t pid, transport::Connection connection)
        : pid_{pid}, connection_{std::move(connection)}
    {
    }

    Process::~Process()
    {
        if (pid() > 0 && !finished_) {
            std::ignore = terminate();
            wait();
        }
    }

    bool Process::send(const command::Command& command) const
    {
        return connection_.send(command);
    }

    bool Process::request_shutdown() const
    {
        return send(command::shutdown_command());
    }

    bool Process::terminate() const
    {
        if (pid_ > 0) {
            if (::kill(pid_, SIGTERM) == 0 || errno == ESRCH) {
                return true;
            }
            return false;
        }

        return true;
    }

    int Process::wait()
    {
        if (finished_)
            return exit_status_;

        int status{0};
        pid_t result;

        do {
            result = ::waitpid(pid_, &status, 0);
        } while (result == -1 && errno == EINTR);

        if (result == pid_ && WIFEXITED(status)) {
            exit_status_ = WEXITSTATUS(status);
        }

        if (errno == ECHILD)
            exit_status_ = 0;

        finished_ = true;
        return exit_status_;
    }

    [[noreturn]] static void start_process(const std::string& executable,
                                           const std::string& name,
                                           const std::vector<std::string>& args,
                                           const fd_t fd)
    {
        static constexpr size_t extra_args = 2 + 2; // 4 when using --socket arg
        const std::string socket_fd = std::to_string(fd);

        std::vector<const char*> argv;
        argv.reserve(args.size() + extra_args);

        argv.push_back(name.c_str());
        LOG_TRACE("name={}", name.c_str());
        for (const auto& arg: args) {
            LOG_TRACE("arg={}", arg.c_str());
            argv.push_back(arg.c_str());
        }

        argv.push_back("--socket");
        argv.push_back(socket_fd.c_str());
        LOG_TRACE("--socket={}", socket_fd.c_str());
        argv.push_back(nullptr);


        // copy fd to STDIN, STDOUT, STDERR
        // dup2(fd, STDOUT_FILENO);
        // dup2(fd, STDIN_FILENO);
        // dup2(fd, STDERR_FILENO);

        execv(executable.c_str(), const_cast<char* const*>(argv.data()));
        LOG_CRIT("Failed to exec {} {}", name, strerror(errno));
        perror(std::format("Failed to exec {}", name).c_str());
        _exit(EXIT_FAILURE);
    }

    Process make_process(ProcessConfig config)
    {
        if (config.executable.empty()) {
            throw std::runtime_error("No executable specified");
        }
        if (config.name.empty()) {
            LOG_TRACE("No executable name. Using executable path {}", config.executable);
            config.name = config.executable;
        }

        auto [fst, snd] = transport::create_connection_pair();

            const pid_t pid = fork();
        if (pid == 0) {
            ::close(fst.fd());
            start_process(config.executable, config.name, config.args, snd.fd());
        }

        if (pid == -1) {
            perror("Failed to fork");
            const auto e = errno;
            LOG_CRIT("Failed to fork. {}({})", strsignal(e), sigabbrev_np(e));
            throw std::runtime_error(std::format("Failed to fork. {}({})", strsignal(e), sigabbrev_np(e)));
        }

        Process process{pid, std::move(fst)};

        return process;
        // snd is closed
    }


} // namespace driver
