//
// Created by andre on 02/11/2026.
//

#ifndef ECE650_A4_EVENT_LOOP_H
#define ECE650_A4_EVENT_LOOP_H
#include <chrono>
#include <condition_variable>
#include <thread>
#include <unordered_map>

#include "router.h"

#include "protocol/connection.h"

namespace driver {

    enum class WorkerId;

    /**
     * @brief Monitors worker connections and routes received commands.
     *
     * @details ConnectionLoop runs a background monitoring thread that periodically checks the
     * registered transport connections for incoming commands. Each connection is associated with a
     * WorkerId, and received commands are forwarded to the Router for handling.
     *
     * The loop stores references to transport::Connection instances and does not take ownership of
     * them. All referenced connections, as well as the Router passed to the constructor, must remain
     * valid for the lifetime of the ConnectionLoop or until monitoring has been stopped.
     *
     * Construction starts the monitoring thread immediately. Calling stop() requests the thread to
     * terminate; destruction of the underlying std::jthread also requests stop and joins the worker.
     *
     * Copy and move operations are disabled because the object owns synchronization state and a running
     * worker thread tied to the referenced Router and registered connections.
     *
     * @note add_connection() is synchronized with the monitoring loop.
     * @warning Concurrent operations on the referenced transport::Connection objects outside this loop
     * must be coordinated by the caller if the connection implementation is not independently
     * thread-safe.
     */
    class ConnectionLoop {
        Router& router_;
        std::chrono::milliseconds monitor_period_;
        std::jthread worker_;
        std::mutex mutex_;
        std::condition_variable_any cv_;
        std::int64_t generation_{};
        std::unordered_map<WorkerId, std::reference_wrapper<const transport::Connection>> connections_;

        void monitor_loop(const std::stop_token& stop);

      public:
        ConnectionLoop(const ConnectionLoop& other) = delete;
        ConnectionLoop(ConnectionLoop&& other) noexcept = delete;
        ConnectionLoop& operator=(const ConnectionLoop& other) = delete;
        ConnectionLoop& operator=(ConnectionLoop&& other) noexcept = delete;
        void stop();

        explicit ConnectionLoop(
                Router& router,
                const std::chrono::milliseconds monitor_period = std::chrono::milliseconds{100})
            : router_{router}, monitor_period_{monitor_period}, worker_{[this](const std::stop_token& stop) {
                  monitor_loop(stop);
              }}
        {
        }

        bool add_connection(WorkerId worker_id, const transport::Connection& connection);
    };

    bool add_connections(const ProcessManager& process_manager, ConnectionLoop& connection_loop);

} // namespace driver

#endif // ECE650_A4_EVENT_LOOP_H
