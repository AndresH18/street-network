//
// Created by andre on 09/11/2026.
//

#ifndef ECE650_A4_COMMAND_DISPATCHER_H
#define ECE650_A4_COMMAND_DISPATCHER_H

#include <functional>
#include <unordered_map>
#include <variant>

#include "command.h"
#include "log.h"

namespace command {
    class CommandDispatcher {
      public:
        using Handler = std::function<void(const Payload&)>;

        void register_handler(const CommandType type, std::function<void()> handler)
        {
            register_handler(type, [handler = std::move(handler)](const Payload&) {
                handler();
            });
        }

        void register_handler(const CommandType type, Handler handler)
        {
            handlers_[type] = std::move(handler);
        }

        template<typename T>
        void register_handler(const CommandType type, std::function<void(const T&)> handler)
        {
            handlers_.try_emplace(type, [handler = std::move(handler)](const Payload& payload) {
                const auto* p = std::get_if<T>(&payload);
                if (!p)
                    throw std::runtime_error("Invalid payload type");

                handler(*p);
            });
        };

        void dispatch(const Command& command)
        {
            const auto it = handlers_.find(command.type);
            if (it == handlers_.end()) {
                LOG_WARNING("No handler registered for command type {}", to_string(command.type));
                return;
            }

            it->second(command.payload);
        }

      private:
        std::unordered_map<CommandType, Handler> handlers_;
    };
} // namespace command


#endif // ECE650_A4_COMMAND_DISPATCHER_H
