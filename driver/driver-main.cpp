//
// Created by andre on 02/11/2026.
//

#include <fcntl.h>
#include <iostream>

#include "protocol/log.h"

#include "connection_loop.h"
#include "process_manager.h"
#include "router.h"

static void sp(const driver::ProcessManager& manager)
{
    auto& connection = manager.get(driver::WorkerId::Sp).connection();

    for (int i = 0; i < 10; ++i) {
        (void) connection.send(
                command::Command{.version = 1,
                                 .request_id = 0,
                                 .type = command::CommandType::GraphUpdate,
                                 .payload = command::GraphUpdate{.vertex_count = 5,
                                                                 .edges = {{0, 1}, {0, 2}, {0, 3}, {0, 4}}}});
        std::this_thread::sleep_for(std::chrono::seconds(2));
        (void) connection.send(command::Command{.version = 1,
                                                .request_id = 0,
                                                .type = command::CommandType::PathRequest,
                                                .payload = command::PathRequest{.start = 1, .end = 3}});
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

static void py(const driver::ProcessManager& manager)
{
    auto& connection = manager.get(driver::WorkerId::Py).connection();

    for (int i = 0; i < 10; ++i) {
        command::Command command{.version = 1,
                                 .request_id = 1,
                                 .type = command::CommandType::AddStreet,
                                 .payload = command::Street{.name = "S1", .points = {{0, 0}, {10, 10}}}};
        (void) connection.send(command);
        std::this_thread::sleep_for(std::chrono::seconds(2));

        command = {.version = 1,
                   .request_id = 2,
                   .type = command::CommandType::AddStreet,
                   .payload = command::Street{.name = "S2", .points = {{0, 10}, {10, 0}}}};
        (void) connection.send(command);
        std::this_thread::sleep_for(std::chrono::seconds(2));

        command = {
                .version = 1,
                .request_id = 3,
                .type = command::CommandType::GenerateGraph,
        };
        (void) connection.send(command);
        std::this_thread::sleep_for(std::chrono::seconds(2));

        command = {.version = 1,
                   .request_id = 4,
                   .type = command::CommandType::RmStreet,
                   .payload = command::RmStreet{.name = "S1"}};
        (void) connection.send(command);
        command = {.version = 1,
                   .request_id = 4,
                   .type = command::CommandType::RmStreet,
                   .payload = command::RmStreet{.name = "S2"}};
        (void) connection.send(command);

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

static void rgen(const driver::ProcessManager& manager)
{
    auto& connection = manager.get(driver::WorkerId::Rgen).connection();

    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::cout << "[driver] Sleeping..." << std::endl;
    }

    std::cout << "[driver] terminating rgen" << std::endl;
    (void) connection.send(command::shutdown_command());
}

int main(int argc, char* argv[])
{
    logger::initialize("log/driver.log");

    // {
    //     int logfd = open("log/log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    //     dup2(logfd, STDERR_FILENO);
    //     close(logfd);
    // }

    driver::ProcessManager manager;

    // Launch processes and establish connection
    manager.start_all();

    driver::Router router{manager};
    driver::ConnectionLoop connection_loop{router};

    driver::add_connections(manager, connection_loop);

    // py(manager);

    // std::this_thread::sleep_for(std::chrono::seconds(30));

    std::string line;
    while (getline(std::cin, line)) {
        if (line.empty()) {
            LOG_INFO("Received empty line, exiting");
            break;
        }

        std::stringstream ss{line};

        int a{-1}, b{-1};
        ss >> a >> b;

        if (a < 0 || b < 0) {
            LOG_WARNING("Invalid input {},{}", a, b);
            continue;
        }

        command::Command command{.version = command::COMMAND_VERSION,
                                 .request_id = 0,
                                 .type = command::CommandType::PathRequest,
                                 .payload = command::PathRequest{a, b}};

        router.handle(driver::WorkerId::Driver, command);
    }

    connection_loop.stop();

    return 0;
}
