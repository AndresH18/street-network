//
// Created by andres on 10/28/26.
//

#ifndef ECE650_A4_SERIALIZER_HPP
#define ECE650_A4_SERIALIZER_HPP
#include "nlohmann/json.hpp"

#include "command.h"

namespace command {
    using nlohmann::json;
    NLOHMANN_JSON_SERIALIZE_ENUM(CommandType,
                                 {{CommandType::AddStreet, "ADD_STREET"},
                                  {CommandType::ModStreet, "MOD_STREET"},
                                  {CommandType::RmStreet, "RM_STREET"},
                                  {CommandType::GenerateGraph, "GENERATE_GRAPH"},
                                  {CommandType::GraphUpdate, "GRAPH_UPDATE"},
                                  {CommandType::PathRequest, "PATH_REQUEST"},
                                  {CommandType::PathResult, "PATH_RESULT"},
                                  {CommandType::Error, "ERROR"},
                                  {CommandType::Shutdown, "SHUTDOWN"},
                                  {CommandType::Ack, "ACK"}})

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Street, name, points);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RmStreet, name)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GraphUpdate, vertex_count, edges);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PathRequest, start, end);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PathResult, success, vertices);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Error, message);

    inline void to_json(json& j, const Command& m)
    {
        j = {{"version", m.version}, {"requestId", m.request_id}, {"type", m.type}};

        // std::visit([&j](const auto& p) {
        //             using T = std::decay_t<decltype(p)>;
        //             if constexpr (std::is_same_v<T, AddStreet>) {
        //                 j["payload"] = p;
        //             }
        //         },
        //         m.payload);
        std::visit(
                [&j]<typename T>(const T& p) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(p)>, std::monostate>)
                        j["payload"] = nullptr;
                    else
                        j["payload"] = p;
                },
                m.payload);
    }

    static Payload get_payload(const json& j, const CommandType type)
    {
        switch (type) {
            case CommandType::None:
                break;
            case CommandType::AddStreet:
            case CommandType::ModStreet:
                return j.get<Street>();
            case CommandType::RmStreet:
                return j.get<RmStreet>();
            case CommandType::GraphUpdate:
                return j.get<GraphUpdate>();
            case CommandType::PathRequest:
                return j.get<PathRequest>();
            case CommandType::PathResult:
                return j.get<PathResult>();
            case CommandType::GenerateGraph:
            case CommandType::Error:
            case CommandType::Shutdown:
            case CommandType::Ack:
                return std::monostate{};
        }

        throw std::runtime_error("Invalid MessageType");
    }

    inline void from_json(const json& j, Command& m)
    {
        m.version = j.at("version").get<int>();
        m.request_id= j.at("requestId").get<int>();
        m.type = j.at("type").get<CommandType>();
        m.payload = get_payload(j["payload"], m.type);
    }


} // namespace transport::command

#endif // ECE650_A4_SERIALIZER_HPP
