#include "Utilities/Utilities.h"

#include <chrono>
#include <format>
#include <iostream>

namespace Engine
{
    namespace Utilities
    {
        void Logging::Log(LogType type, LogLevel level, const char* message)
        {
            // Translate enum values to readable strings.
            const char* l_LevelString = "";
            const char* l_TypeString = "";

            switch (type)
            {
            case LogType::CORE:
                l_TypeString = "ENGINE";
                break;

            case LogType::CLIENT:
                l_TypeString = "SANDBOX";
                break;
            }

            switch (level)
            {
            case LogLevel::INFO:
                l_LevelString = "INFO";
                break;

            case LogLevel::WARNING:
                l_LevelString = "WARNING";
                break;

            case LogLevel::ERROR:
                l_LevelString = "ERROR";
                break;
            }

            // Output the formatted message with timestamp and category.
            std::cout << std::format("[{:%H:%M:%S}] {}: {}", std::chrono::zoned_time{ std::chrono::current_zone(),
                            std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()) }, l_TypeString, message) << std::endl;
        }

        void Logging::LogInfo(LogType type, const char* message)
        {
            // Public wrapper for informational messages.
            Log(type, LogLevel::INFO, message);
        }

        void Logging::LogWarning(LogType type, const char* message)
        {
            // Public wrapper for warnings.
            Log(type, LogLevel::WARNING, message);
        }

        void Logging::LogError(LogType type, const char* message)
        {
            // Public wrapper for errors.
            Log(type, LogLevel::ERROR, message);
        }

        void Logging::Assert(LogType type, bool condition, const char* message)
        {
            // If the assertion fails, log the error and abort.
            if (!condition)
            {
                LogError(type, message);
                std::abort();
            }
        }
    }
}