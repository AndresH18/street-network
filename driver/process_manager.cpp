//
// Created by andre on 30/10/2026.
//

#include "process_manager.h"

#include <format>
#include <stdexcept>
#include <string>

#include "process.h"
#include "router.h"

#include "protocol/log.h"

namespace driver {
    void ProcessManager::start_all()
    {
        processes_.emplace(WorkerId::Sp,
                           make_process({.name = to_string(WorkerId::Sp), .executable = SP_EXECUTABLE}));

        processes_.emplace(WorkerId::Py,
                           make_process({.name = to_string(WorkerId::Py), .executable = PY_EXECUTABLE}));

        processes_.emplace(WorkerId::Rgen,
                           make_process({
                                   .name = to_string(WorkerId::Rgen),
                                   .executable = RGEN_EXECUTABLE,
                           }));
    }

    const Process& ProcessManager::get(const WorkerId id) const
    {
        LOG_TRACE("Getting process for worker {}", to_string(id));
        const auto it = processes_.find(id);
        if (it == processes_.end()) {
            LOG_ERROR("Worker {} not found", to_string(id));
            throw std::out_of_range(std::format("Worker {} not found", to_string(id)));
        }

        return it->second;
    }
} // namespace driver
