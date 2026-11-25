//
// Created by andres on 11/1/26.
//

#include "router.h"

#include <format>
#include <iostream>
#include <variant>

#include "nlohmann/json.hpp"
#include "protocol/serializer.h"

#include "process_manager.h"
#include "protocol/log.h"

namespace driver {
    void Router::forward(const WorkerId worker, const command::Command& command) const
    {
        if (worker == WorkerId::Driver) {
            LOG_TRACE("command for driver. Ignore and continue");
            return;
        }

        LOG_DEBUG("forwarding command ({}) to worker {}", to_string(command.type), to_string(worker));
        if (!process_manager_.get(worker).send(command)) {
            LOG_ERROR("Could not send command to {}", to_string(worker));
        }
    }

    void Router::handle(const WorkerId source, const command::Command& command)
    {
        std::lock_guard lock{mutex_};
        LOG_TRACE("command ({}) from worker {}", to_string(command.type), to_string(source));
        nlohmann::json j = command;
        std::clog << "[Router] Received response from " << to_string(source) << " " << j.dump() << std::endl;
        switch (command.type) {
            case command::CommandType::AddStreet:
            case command::CommandType::ModStreet:
            case command::CommandType::RmStreet:
            case command::CommandType::GenerateGraph:
                // only rgen should send these
                if (source != WorkerId::Rgen)
                    return;
                forward(WorkerId::Py, command);
                break;
            case command::CommandType::GraphUpdate:
                if (source != WorkerId::Py)
                    return;
                forward(WorkerId::Sp, command);
                break;
            case command::CommandType::PathResult: {
                if (source != WorkerId::Sp)
                    return;
                auto& path_result = std::get<command::PathResult>(command.payload);

                std::cout << "[Router] Path: " << path_result << std::endl;
                break;
            }
            case command::CommandType::PathRequest:
                if (source != WorkerId::Driver)
                    return;
                forward(WorkerId::Sp, command);
                break;
            case command::CommandType::Error:
                LOG_ERROR("Received error from {}", to_string(source));
                break;
            case command::CommandType::None:
            case command::CommandType::Shutdown:
            case command::CommandType::Ack:
                break;
        }
    }
} // namespace driver
