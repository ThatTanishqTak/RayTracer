#include "Utilities/Logging.h"

#include <chrono>
#include <format>
#include <iostream>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

namespace Engine
{
    namespace Utilities
    {
        // s_ConsoleHandle caches the console handle on Windows so it only has to be
        // looked up once. Not used on other platforms.
#ifdef _WIN32
        static HANDLE s_ConsoleHandle = nullptr;
#endif

        /**\brief Apply a color based on the log level.*/
        static void SetColor(Logging::LogLevel level)
        {
#ifdef _WIN32
            // Lazily acquire the console handle the first time we log.
            if (s_ConsoleHandle == nullptr)
            {
                s_ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            }

            WORD l_Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // default gray
            switch (level)
            {
            case Logging::LogLevel::_TRACE:
                l_Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; // bright white
                break;
            case Logging::LogLevel::_INFO:
                l_Attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY; // bright green
                break;
            case Logging::LogLevel::_WARNING:
                l_Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; // yellow
                break;
            case Logging::LogLevel::_ERROR:
                l_Attributes = FOREGROUND_RED | FOREGROUND_INTENSITY; // bright red
                break;
            }

            SetConsoleTextAttribute(s_ConsoleHandle, l_Attributes);
#else
            const char* l_Code = "";
            switch (level)
            {
            case Logging::LogLevel::_TRACE:
                l_Code = "\x1B[37m"; // white
                break;
            case Logging::LogLevel::_INFO:
                l_Code = "\x1B[32m"; // Green
                break;
            case Logging::LogLevel::_WARNING:
                l_Code = "\x1B[33m"; // Yellow
                break;
            case Logging::LogLevel::_ERROR:
                l_Code = "\x1B[31m"; // Red
                break;
            }

            std::cout << l_Code;
#endif
        }

        /**\brief Reset the console color to its default.*/
        static void ResetColor()
        {
#ifdef _WIN32
            SetConsoleTextAttribute(s_ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
            std::cout << "\x1B[0m";
#endif
        }

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
            case LogLevel::_TRACE:
                l_LevelString = "TRACE";
                break;
            case LogLevel::_INFO:
                l_LevelString = "INFO";
                break;

            case LogLevel::_WARNING:
                l_LevelString = "WARNING";
                break;

            case LogLevel::_ERROR:
                l_LevelString = "ERROR";
                break;
            }

            // Apply level-based color, output the message, then reset the color.
            SetColor(level);
            std::cout << std::format("[{:%H:%M:%S}] {}: {}", std::chrono::zoned_time{ std::chrono::current_zone(),
                std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()) }, l_TypeString, message) << std::endl;

            ResetColor();
        }
    }
}