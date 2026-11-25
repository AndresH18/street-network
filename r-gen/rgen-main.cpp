#include <atomic>
#include <ranges>
#include <thread>
#include <vector>

#include "protocol/command_dispatcher.h"
#include "protocol/connection.h"
#include "protocol/log.h"
#include "protocol/worker.h"

#include "generator.h"
#include "random.h"

static void send(const std::vector<rgen::Street>& previous,
                 const std::vector<rgen::Street>& current,
                 const transport::Connection& connection);

int main(int argc, char* argv[])
{
    logger::initialize("log/rgen.log");

    try {
        transport::Connection connection{transport::get_socket(argc, argv)};
        command::CommandDispatcher dispatcher; // rgen only receives Shutdown command
        Worker worker{connection, dispatcher};

        std::jthread w_thread{[&connection](const std::stop_token& stop_token) {
            std::vector<rgen::Street> previous;
            while (!stop_token.stop_requested()) {
                auto current = rgen::generate_valid_specification();

                send(previous, current, connection);

                previous = std::move(current);

                const auto wait_time = static_cast<unsigned int>(
                        rgen::random::next_int(rgen::MIN_WAIT, rgen::DEFAULT_MAX_WAIT));
                std::this_thread::sleep_for(std::chrono::seconds{wait_time});
            }
        }};

        worker.run();
        w_thread.request_stop();
    } catch (const std::exception& e) {
        LOG_CRIT("Exception: {}", e.what());
    }
}

void send(const std::vector<rgen::Street>& previous,
          const std::vector<rgen::Street>& current,
          const transport::Connection& connection)
{
    unsigned int i = 0;
    command::Command command;

    // remove previous
    LOG_TRACE("Removing current specification");
    for (const auto& street: previous) {
        command = {.request_id = i++,
                   .type = command::CommandType::RmStreet,
                   .payload = command::RmStreet{.name = street.name()}};

        if (connection.send(command)) {
            LOG_TRACE("Sent {} command", to_string(command.type));
        } else {
            LOG_WARNING("Failed to send command");
            return;
        }
    }

    LOG_TRACE("Sending current specification");
    for (const auto& street: current) {

        // convert points to int pair
        auto points_transform = street.points() | std::views::transform([](const auto& point) {
                                    return std::pair<int, int>{point.x(), point.y()};
                                });

        const std::vector points(points_transform.begin(), points_transform.end());

        command = {.request_id = i++,
                   .type = command::CommandType::AddStreet,
                   .payload = command::Street{.name = street.name(), .points = points}};

        if (connection.send(command)) {
            LOG_TRACE("Sent {} command", to_string(command.type));
        } else {
            LOG_WARNING("Failed to send command");
            return;
        }
    }

    LOG_TRACE("Send Generate graph");
    command = {.request_id = i++, .type = command::CommandType::GenerateGraph};
    if (!connection.send(command))
        LOG_WARNING("Failed to send command");

    LOG_TRACE("Sent {} command", to_string(command.type));
}
