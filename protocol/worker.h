//
// Created by andres on 10/29/26.

#ifndef ECE650_A4_WORKER_H
#define ECE650_A4_WORKER_H

#include <cstring>
#include <iostream>
#include <stop_token>
#include <sys/poll.h>

#include "command.h"
#include "command_dispatcher.h"
#include "connection.h"
#include "log.h"

/**
 * @class Worker
 * @brief Runs a command-processing loop over a transport connection.
 *
 * @details
 * Worker owns the control flow for receiving commands from a
 * transport::Connection and forwarding them to a command::CommandDispatcher.
 * It keeps non-owning references to both objects; therefore, the connection and
 * dispatcher must remain valid for the entire lifetime of the Worker.
 *
 * The worker registers a handler for command::CommandType::Shutdown during
 * construction. When a shutdown command is dispatched, the handler requests the
 * worker loop to stop.
 *
 * The worker can be stopped explicitly by calling stop(), or indirectly by
 * dispatching a shutdown command through the associated dispatcher.
 *
 * @note
 * The class does not own the underlying connection or dispatcher.
 */
class Worker {
    transport::Connection& connection_;
    command::CommandDispatcher& dispatcher_;
    std::atomic_bool running_{false};

  public:
    /**
     * @brief Starts the worker command-processing loop.
     *
     * @details
     * If the worker is already running, this function returns immediately. Otherwise,
     * it marks the worker as running and waits for input on the associated transport
     * connection file descriptor using poll().
     *
     * The loop periodically wakes to check the running state, ignores signal
     * interruptions, receives commands from the connection when input is available,
     * and forwards successfully received commands to the command dispatcher.
     *
     * The loop exits when stop() clears the running state, when polling fails with
     * an unrecoverable error, or when the connection reports an invalid descriptor,
     * hangup, or error condition.
     *
     * @note
     * This function blocks the calling thread until the worker loop terminates.
     *
     * @note
     * Calling this function while the worker is already running has no effect.
     */
    void run();

    explicit Worker(transport::Connection& connection, command::CommandDispatcher& dispatcher)
        : connection_{connection}, dispatcher_{dispatcher}
    {
        dispatcher_.register_handler(command::CommandType::Shutdown, [this] {
            this->stop();
        });
    }

    /**
     * @brief Requests the worker command-processing loop to stop.
     *
     * @details
     * Logs a trace message and clears the running state. If run() is currently
     * active, the worker loop observes the cleared state and exits on its next
     * iteration or wake-up.
     *
     * Calling this function when the worker is not running has no additional effect.
     */
    void stop()
    {
        LOG_TRACE("Stopping worker");
        running_ = false;
    }

  protected:
    [[nodiscard]] const transport::Connection& connection() const
    {
        return connection_;
    }
};

inline void Worker::run()
{
    // set to true and returns previous value
    //  if already true, return
    if (running_.exchange(true))
        return;

    std::cout << "Starting worker" << std::endl;

    LOG_TRACE("Starting worker");

    pollfd pdf{
            .fd = connection_.fd(),
            .events = POLLIN,
            .revents = 0,
    };

    while (running_) {
        const auto n = ::poll(&pdf, 1, 100);
        if (n == 0) // timed-out
            continue;
        if (n == -1) {
            const auto err = errno;
            if (err == EINTR) // interrupted by signal
                continue;
            LOG_ERROR("poll failed with error: {}", strerror(err));
            break;
        }

        if (pdf.revents & (POLLNVAL | POLLHUP | POLLERR)) {
            // Invalid FD, hangup, or error
            LOG_ERROR("Invalid FD, hangup, or error {:#06x}", pdf.revents);
            break;
        }

        if (!(pdf.fd & POLLIN))
            continue;

        if (command::Command command; connection_.recv(command)) {
            dispatcher_.dispatch(command);
        }
    }
}


#endif // ECE650_A4_WORKER_H
