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

			std::cout << std::format("[{:%H:%M:%S}] {}: {}", std::chrono::zoned_time{ std::chrono::current_zone(),
					std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()) }, l_TypeString, message) << std::endl;
		}

		void Logging::LogInfo(LogType type, const char* message)
		{
			Log(type, LogLevel::INFO, message);
		}

		void Logging::LogWarning(LogType type, const char* message)
		{
			Log(type, LogLevel::WARNING, message);
		}

		void Logging::LogError(LogType type, const char* message)
		{
			Log(type, LogLevel::ERROR, message);
		}

		void Logging::Assert(LogType type, bool condition, const char* message)
		{
			if (!condition)
			{
				LogError(type, message);
				std::abort();
			}
		}
	}
}