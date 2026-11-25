//
// Created by andre on 09/11/2026.
//

#ifndef ECE650_A4_COMMAND_H
#define ECE650_A4_COMMAND_H

#include <ostream>
#include <string>
#include <variant>
#include <vector>

namespace command {
    static constexpr int COMMAND_VERSION = 1;

    enum class CommandType {
        None,
        AddStreet,
        ModStreet,
        RmStreet,
        GenerateGraph,
        GraphUpdate,
        PathRequest,
        PathResult,
        Error,
        Shutdown,
        Ack,
    };

    inline const char* to_string(const CommandType e)
    {
        switch (e) {
            case CommandType::None:
                return "NONE";
            case CommandType::AddStreet:
                return "ADD_STREET";
            case CommandType::ModStreet:
                return "MOD_STREET";
            case CommandType::RmStreet:
                return "RM_STREET";
            case CommandType::GenerateGraph:
                return "GENERATE_GRAPH";
            case CommandType::GraphUpdate:
                return "GRAPH_UPDATE";
            case CommandType::PathRequest:
                return "PATH_REQUEST";
            case CommandType::PathResult:
                return "PATH_RESULT";
            case CommandType::Error:
                return "ERROR";
            case CommandType::Shutdown:
                return "SHUTDOWN";
            case CommandType::Ack:
                return "ACK";
            default:
                return "UNKNOWN";
        }
    }

    struct Street {
        std::string name;
        std::vector<std::pair<int, int>> points;
    };

    struct RmStreet {
        std::string name;
    };

    struct GraphUpdate {
        int vertex_count;
        std::vector<std::pair<int, int>> edges;
    };

    struct PathRequest {
        int start;
        int end;
    };

    struct PathResult {
        bool success = false;
        std::vector<int> vertices;

        friend std::ostream& operator<<(std::ostream& os, const PathResult& obj)
        {
            if (obj.success) {
                os << "vertices: ";
                bool first = true;
                for (const auto& vertex: obj.vertices) {
                    if (!first)
                        os << '-';
                    os << vertex;
                    first = false;
                }

                return os;
            }
            return os << "No path";
        }
    };

    struct Error {
        std::string message;
    };

    using Payload = std::variant<std::monostate, Street, RmStreet, GraphUpdate, PathRequest, PathResult>;

    struct Command {
        unsigned int version{COMMAND_VERSION};
        unsigned int request_id;
        CommandType type;
        Payload payload;
    };

    inline Command shutdown_command(const unsigned int version = COMMAND_VERSION,
                                    const unsigned int requestId = 0)
    {
        return Command{.version = version, .request_id = requestId, .type = CommandType::Shutdown};
    }

    inline Command error_command(const unsigned int version = COMMAND_VERSION,
                                 const unsigned int requestId = 0)
    {
        return Command{.version = version, .request_id = requestId, .type = CommandType::Error};
    }

} // namespace command

#endif // ECE650_A4_COMMAND_H
