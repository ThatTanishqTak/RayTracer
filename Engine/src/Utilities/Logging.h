#pragma once

#include <cstdlib>
#include <format>
#include <string>
#include <string_view>

namespace Engine
{
    namespace Utilities
    {
        /**
         *\brief Helper class providing basic logging facilities for the engine and client.
         */
        class Logging
        {
        public:
            /**\brief Identifies the origin of the log message.*/
            enum class LogType
            {
                CORE,
                CLIENT
            };

            /**\brief Severity levels supported by the logger.*/
            enum class LogLevel
            {
                _TRACE,
                _INFO,
                _WARNING,
                _ERROR
            };

            /**\brief Log a trace message.*/
            template<typename... Args>
            static void LogTrace(LogType type, std::string_view format, Args&&... a_Args)
            {
                Log(type, LogLevel::_TRACE, format, std::forward<Args>(a_Args)...);
            }

            /**\brief Log an informational message.*/
            template<typename... Args>
            static void LogInfo(LogType type, std::string_view format, Args&&... a_Args)
            {
                Log(type, LogLevel::_INFO, format, std::forward<Args>(a_Args)...);
            }

            /**\brief Log a warning message.*/
            template<typename... Args>
            static void LogWarning(LogType type, std::string_view format, Args&&... a_Args)
            {
                Log(type, LogLevel::_WARNING, format, std::forward<Args>(a_Args)...);
            }

            /**\brief Log an error message.*/
            template<typename... Args>
            static void LogError(LogType type, std::string_view format, Args&&... a_Args)
            {
                Log(type, LogLevel::_ERROR, format, std::forward<Args>(a_Args)...);
            }

            /**\brief Abort execution if condition is false and report the error.*/
            template<typename... Args>
            static void Assert(LogType type, bool condition, std::string_view format, Args&&... a_Args)
            {
                if (!condition)
                {
                    Log(type, LogLevel::_ERROR, format, std::forward<Args>(a_Args)...);
                    std::abort();
                }
            }

        private:
            /**\brief Shared logging implementation for all severity levels.*/
            static void Log(LogType type, LogLevel level, const std::string& message);

            template<typename... Args>
            static void Log(LogType type, LogLevel level, std::string_view format, Args&&... a_Args)
            {
                std::string l_Message = std::vformat(format, std::make_format_args(a_Args...));
                Log(type, level, l_Message);
            }
        };
    }

#if _DEBUG
#define RAY_ASSERT(condition, ...) do { \
    /* Assert with CORE log type; do-while wraps for safe single-statement usage */ \
    ::Engine::Utilities::Logging::Assert(::Engine::Utilities::Logging::LogType::CORE, (condition), __VA_ARGS__); \
} while (0)

#define RAY_CORE_TRACE(...) ::Engine::Utilities::Logging::LogTrace(::Engine::Utilities::Logging::LogType::CORE, __VA_ARGS__)
#define RAY_CORE_INFO(...) ::Engine::Utilities::Logging::LogInfo(::Engine::Utilities::Logging::LogType::CORE, __VA_ARGS__)
#define RAY_CORE_WARNING(...) ::Engine::Utilities::Logging::LogWarning(::Engine::Utilities::Logging::LogType::CORE, __VA_ARGS__)
#define RAY_CORE_ERROR(...) ::Engine::Utilities::Logging::LogError(::Engine::Utilities::Logging::LogType::CORE, __VA_ARGS__)

#define RAY_TRACE(...) ::Engine::Utilities::Logging::LogTrace(::Engine::Utilities::Logging::LogType::CLIENT, __VA_ARGS__)
#define RAY_INFO(...) ::Engine::Utilities::Logging::LogInfo(::Engine::Utilities::Logging::LogType::CLIENT, __VA_ARGS__)
#define RAY_WARNING(...) ::Engine::Utilities::Logging::LogWarning(::Engine::Utilities::Logging::LogType::CLIENT, __VA_ARGS__)
#define RAY_ERROR(...) ::Engine::Utilities::Logging::LogError(::Engine::Utilities::Logging::LogType::CLIENT, __VA_ARGS__)
#else
#define RAY_ASSERT(condition, ...)
#define RAY_CORE_TRACE(...)
#define RAY_CORE_INFO(...)
#define RAY_CORE_WARNING(...)
#define RAY_CORE_ERROR(...)

#define RAY_TRACE(...)
#define RAY_INFO(...)
#define RAY_WARNING(...)
#define RAY_ERROR(...)
#endif
}