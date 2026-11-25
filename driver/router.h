//
// Created by andres on 11/1/26.
//

#ifndef ECE650_A4_ROUTER_H
#define ECE650_A4_ROUTER_H
#include "process.h"

#include <mutex>

namespace driver {
    enum class WorkerId;
    class ProcessManager;

    class Router {
        ProcessManager& process_manager_;
        std::mutex mutex_;


        void forward(WorkerId worker, const command::Command& command) const;

      public:
        explicit Router(ProcessManager& manager)
            : process_manager_{manager}
        {
        }

        void handle(WorkerId source, const command::Command& command);
    };


} // namespace driver

#endif // ECE650_A4_ROUTER_H
