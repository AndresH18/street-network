//
// Created by andre on 30/10/2026.
//

#ifndef ECE650_A4_PROCESS_MANAGER_H
#define ECE650_A4_PROCESS_MANAGER_H
#include <string>
#include <unordered_map>

#include <vector>

namespace driver {
    class ConnectionLoop;
    // forward-declaration
    enum class WorkerId;
    class Process;

    class ProcessManager {
        std::unordered_map<WorkerId, Process> processes_;

      public:
        void start_all();

        const Process& get(WorkerId id) const;

        friend bool add_connections(const ProcessManager& process_manager, ConnectionLoop& connection_loop);
    };

    enum class WorkerId {
        Driver,
        Rgen,
        Py,
        Sp,
    };

    inline const char* to_string(const WorkerId e)
    {
        switch (e) {
            case WorkerId::Driver:
                return "Driver";
            case WorkerId::Rgen:
                return "Rgen";
            case WorkerId::Py:
                return "Py";
            case WorkerId::Sp:
                return "Sp";
            default:
                return "unknown";
        }
    }


} // namespace driver

#endif // ECE650_A4_PROCESS_MANAGER_H
