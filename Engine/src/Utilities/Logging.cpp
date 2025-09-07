#include "Utilities/Logging.h"

#include <chrono>
#include <format>
#include <iostream>
#include <string>

namespace Engine
{
    namespace Utilities
    {
        void Logging::Log(LogType type, LogLevel level, const std::string& message)
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
    }
}