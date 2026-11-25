//
// Created by andre on 02/11/2026.
//

#include "connection_loop.h"

#include <cstring>
#include <poll.h>

#include "process_manager.h"
#include "protocol/command.h"
#include "protocol/log.h"

namespace driver {

    void ConnectionLoop::monitor_loop(const std::stop_token& stop)
    {
        while (!stop.stop_requested()) {
            std::vector<struct pollfd> fds;
            std::vector<WorkerId> workers;
            {
                std::unique_lock lock(mutex_);
                auto generation = generation_;

                LOG_TRACE("Waiting for event. Generation: {}", generation);

                // wait monitor_period or until notified (with predicate == true), release mutex while
                // waiting.
                cv_.wait_for(lock, stop, monitor_period_, [this, generation] {
                    return generation != generation_;
                });

                if (stop.stop_requested())
                    break;

                const auto connections_size = connections_.size();
                LOG_TRACE("Found {} connections", connections_size);
                fds.reserve(connections_size);
                for (const auto& [worker, connection]: connections_) {
                    fds.push_back({.fd = connection.get().fd(), .events = POLLIN, .revents = 0});
                    workers.push_back(worker);
                }
            }

            // wait for event
            LOG_TRACE("Polling connections");
            const int result = ::poll(fds.data(), fds.size(), 10);
            LOG_TRACE("Poll returned {}", result);

            if (stop.stop_requested())
                break;

            if (result < 0) {
                if (errno == EINTR) {
                    LOG_TRACE("Poll interrupted. Continue");
                    continue;
                }

                LOG_ERROR("Poll error: {}", strerror(errno));
                break;
            }
            if (result == 0)
                continue;

            for (size_t i = 0; i < fds.size(); ++i) {
                const auto& fd = fds[i];
                const auto& worker = workers[i];

                if (fd.revents == 0) {
                    LOG_TRACE("Connection {} has no rvents", to_string(worker));
                    continue;
                }
                if (fd.revents & POLLIN) {
                    std::lock_guard lock(mutex_);

                    const auto it = connections_.find(worker);
                    if (it == connections_.end())
                        continue;

                    const auto& connection = it->second.get();
                    if (command::Command command; connection.recv(command)) {
                        router_.handle(worker, command);
                    } else {
                        LOG_ERROR("Could not receive command from worker {}", to_string(worker));
                    }
                }

                // if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
                // {
                //     // Handle connection errors or disconnection.
                //     handle_worker_disconnect(worker_id);
                // }
            }
        }
    }

    void ConnectionLoop::stop()
    {
        LOG_TRACE("Stopping loop");
        worker_.request_stop();
    }

    bool ConnectionLoop::add_connection(const WorkerId worker_id, const transport::Connection& connection)
    {
        LOG_TRACE("Adding connection for worker {}", to_string(worker_id));
        std::lock_guard lock{mutex_};
        const auto [fst, snd] = connections_.try_emplace(worker_id, std::cref(connection));
        return snd;
    }

    bool add_connections(const ProcessManager& process_manager,  ConnectionLoop& connection_loop)
    {
        for (const auto& [worker, process] : process_manager.processes_) {
            connection_loop.add_connection(worker, process.connection());
        }
        return true;
    }
} // namespace driver
