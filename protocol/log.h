//
// Created by andre on 02/11/2026.
//

#ifndef ECE650_A4_LOG_H
#define ECE650_A4_LOG_H
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <source_location>
#include <string>

namespace logger {
    inline std::ofstream logfile;
    inline std::mutex mutex;

    inline bool initialize(const std::filesystem::path& filename)
    {
        std::lock_guard lock{mutex};

        if (filename.has_parent_path()) {
            std::filesystem::create_directories(filename.parent_path());
        }

        if (logfile.is_open()) {
            logfile.close();
        }

        logfile.open(filename, std::ios::app);

        return logfile.is_open();
    }

    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical,
        None,
    };

    inline auto log_level = LogLevel::None;

    static const char* to_string(const LogLevel e)
    {
        switch (e) {
            case LogLevel::Trace:
                return "TRACE";
            case LogLevel::Debug:
                return "DEBUG";
            case LogLevel::Info:
                return "INFO";
            case LogLevel::Warning:
                return "WARNING";
            case LogLevel::Error:
                return "ERROR";
            case LogLevel::Critical:
                return "CRITICAL";
            case LogLevel::None:
                return "NONE";
        }

        return "unknown";
    }

    inline void log_message(const LogLevel level,
                            const std::string_view message,
                            const std::source_location location = std::source_location::current())
    {
        std::lock_guard lock{mutex};

        if (!logfile.is_open()) {
            std::cerr << "Error: Could not open log file.\n";
            return;
        }

        const auto now = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(now);

        std::tm localTime{};

#ifdef _WIN32
        localtime_s(&localTime, &time);
#else
        localtime_r(&time, &localTime);
#endif

        logfile << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << ' ' << to_string(level) << " ["
                << location.function_name() << "] " << message << '\n';
        logfile.flush();
    }

    template<typename... Args>
    void log(const LogLevel level,
             const std::source_location location,
             const std::format_string<Args...> fmt,
             Args&&... args)
    {
        log_message(level, std::format(fmt, std::forward<Args>(args)...), location);
    }

    // template<typename... Args>
    // void trace(const std::source_location location, const std::format_string<Args...> fmt, Args&&... args)
    // {
    //     log(LogLevel::Trace, location, fmt, std::forward<Args>(args)...);
    // }
    // template<typename... Args>
    // void debug(const std::source_location location, const std::format_string<Args...> fmt, Args&&... args)
    // {
    //     log(LogLevel::Debug, location, fmt, std::forward<Args>(args)...);
    // }
    // template<typename... Args>
    // void info(const std::source_location location, const std::format_string<Args...> fmt, Args&&... args)
    // {
    //     log(LogLevel::Info, location, fmt, std::forward<Args>(args)...);
    // }
    // template<typename... Args>
    // void warning(const std::source_location location, const std::format_string<Args...> fmt, Args&&...
    // args)
    // {
    //     log(LogLevel::Warning, location, fmt, std::forward<Args>(args)...);
    // }
    // template<typename... Args>
    // void error(const std::source_location location, const std::format_string<Args...> fmt, Args&&... args)
    // {
    //     log(LogLevel::Error, location, fmt, std::forward<Args>(args)...);
    // }
    // template<typename... Args>
    // void critical(const std::source_location location, const std::format_string<Args...> fmt, Args&&...
    // args)
    // {
    //     log(LogLevel::Critical, location, fmt, std::forward<Args>(args)...);
    // }

} // namespace logger

#ifndef LOG_COMPILE_LEVEL
#ifdef NDEBUG
#define LOG_COMPILE_LEVEL logger::LogLevel::Info
#else
#define LOG_COMPILE_LEVEL logger::LogLevel::Trace
#endif
#endif

#define LOG_TRACE(...)                                                                                       \
    do {                                                                                                     \
        if constexpr (LOG_COMPILE_LEVEL <= logger::LogLevel::Trace)                                          \
            logger::log(logger::LogLevel::Trace, std::source_location::current(), __VA_ARGS__);              \
    } while (false)

#define LOG_DEBUG(...)                                                                                       \
    do {                                                                                                     \
        if constexpr (LOG_COMPILE_LEVEL <= logger::LogLevel::Debug)                                          \
            logger::log(logger::LogLevel::Debug, std::source_location::current(), __VA_ARGS__);              \
    } while (false)

#define LOG_INFO(...)                                                                                        \
    do {                                                                                                     \
        if constexpr (LOG_COMPILE_LEVEL <= logger::LogLevel::Info)                                           \
            logger::log(logger::LogLevel::Info, std::source_location::current(), __VA_ARGS__);               \
    } while (false)

#define LOG_WARNING(...)                                                                                     \
    do {                                                                                                     \
        if constexpr (LOG_COMPILE_LEVEL <= logger::LogLevel::Warning)                                        \
            logger::log(logger::LogLevel::Warning, std::source_location::current(), __VA_ARGS__);            \
    } while (false)

#define LOG_ERROR(...)                                                                                       \
    do {                                                                                                     \
        if constexpr (LOG_COMPILE_LEVEL <= logger::LogLevel::Error)                                          \
            logger::log(logger::LogLevel::Error, std::source_location::current(), __VA_ARGS__);              \
    } while (false)

#define LOG_CRIT(...)                                                                                        \
    do {                                                                                                     \
        if constexpr (LOG_COMPILE_LEVEL <= logger::LogLevel::Critical)                                       \
            logger::log(logger::LogLevel::Critical, std::source_location::current(), __VA_ARGS__);           \
    } while (false)

#endif // ECE650_A4_LOG_H
