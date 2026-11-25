//
// Created by andre on 04/11/2026.
//

#include <algorithm>
#include <getopt.h>
#include <iostream>
#include <optional>
#include <string>


#include "graph.h"
#include "protocol/command_dispatcher.h"
#include "protocol/connection.h"
#include "protocol/log.h"
#include "protocol/worker.h"

static void handle_graph_update(const command::GraphUpdate& graph_update, std::optional<sp::Graph>& graph);

static void handle_path_request(const std::optional<sp::Graph>& graph,
                                const transport::Connection& connection,
                                const command::PathRequest& path_request);

int main(int argc, char* argv[])
{
    logger::initialize("log/short-path.log");
    try {
        int socket = transport::get_socket(argc, argv);
        if (socket < 0) {
            LOG_CRIT("Invalid socket {}", socket);
            return EXIT_FAILURE;
        }

        std::optional<sp::Graph> graph;

        transport::Connection connection{socket};
        command::CommandDispatcher dispatcher;

        dispatcher.register_handler<command::GraphUpdate>(command::CommandType::GraphUpdate,
                                                          [&graph](const command::GraphUpdate& graph_update) {
                                                              handle_graph_update(graph_update, graph);
                                                          });
        dispatcher.register_handler<command::PathRequest>(
                command::CommandType::PathRequest,
                [&connection, &graph](const command::PathRequest& path_request) {
                    handle_path_request(graph, connection, path_request);
                });

        Worker worker{connection, dispatcher};
        worker.run();
    } catch (const std::exception& e) {
        LOG_CRIT("Exception: {}", e.what());
    }
}

static void handle_graph_update(const command::GraphUpdate& graph_update, std::optional<sp::Graph>& graph)
{
    std::clog << "[SP] Received graph update" << std::endl;
    // py sends zero-based indexes
    graph.emplace(graph_update.vertex_count, graph_update.edges);
    std::cout << "[SP] Graph: " << *graph << std::endl;
}

static void handle_path_request(const std::optional<sp::Graph>& graph,
                                const transport::Connection& connection,
                                const command::PathRequest& path_request)
{
    std::clog << "[SP] Received path request" << std::endl;
    (void) connection.send(
            command::Command{.version = 1, .request_id = 0, .type = command::CommandType::Ack});

    if (!graph) {
        LOG_ERROR("No graph available");
        (void) connection.send(command::error_command());
        return;
    }

    if (graph->valid_vertex(path_request.start) && graph->valid_vertex(path_request.end)) {
        const auto path = graph->shortest_path(path_request.start, path_request.end);

        command::Command result{.version = command::COMMAND_VERSION,
                                .type = command::CommandType::PathResult,
                                .payload = command::PathResult{.success = path.has_value()}};

        if (path) {
            get<command::PathResult>(result.payload).vertices = *path;
        }

        (void) connection.send(result);
    }
}
