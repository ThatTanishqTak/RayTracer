#pragma once

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
                INFO,
                WARNING,
                ERROR
            };

            /**\brief Log an informational message.*/
            static void LogInfo(LogType type, const char* message);
            /**\brief Log a warning message.*/
            static void LogWarning(LogType type, const char* message);
            /**\brief Log an error message.*/
            static void LogError(LogType type, const char* message);

            /**\brief Abort execution if condition is false and report the error.*/
            static void Assert(LogType type, bool condition, const char* message);

        private:
            /**\brief Shared logging implementation for all severity levels.*/
            static void Log(LogType type, LogLevel level, const char* message);
        };
    }

#if _DEBUG
#define RAY_ASSERT(condition, message) ::Engine::Utilities::Logging::Assert(condition, message)

#define RAY_CORE_INFO(message) ::Engine::Utilities::Logging::LogInfo(::Engine::Utilities::Logging::LogType::CORE, message)
#define RAY_CORE_WARNING(message) ::Engine::Utilities::Logging::LogWarning(::Engine::Utilities::Logging::LogType::CORE, message)
#define RAY_CORE_ERROR(message) ::Engine::Utilities::Logging::LogError(::Engine::Utilities::Logging::LogType::CORE, message)

#define RAY_INFO(message) ::Engine::Utilities::Logging::LogInfo(::Engine::Utilities::Logging::LogType::CLIENT, message)
#define RAY_WARNING(message) ::Engine::Utilities::Logging::LogWarning(::Engine::Utilities::Logging::LogType::CLIENT, message)
#define RAY_ERROR(message) ::Engine::Utilities::Logging::LogError(::Engine::Utilities::Logging::LogType::CLIENT, message)
#else
#define RAY_ASSERT(condition, message)
#define RAY_CORE_INFO(message)
#define RAY_CORE_WARNING(message)
#define RAY_CORE_ERROR(message)

#define RAY_INFO(message)
#define RAY_WARNING(message)
#define RAY_ERROR(message)
#endif

}