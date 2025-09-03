#pragma once

namespace Engine
{
	namespace Utilities
	{
		class Logging
		{
		public:
			enum class LogType
			{
				CORE,
				CLIENT
			};

			enum class LogLevel
			{
				INFO,
				WARNING,
				ERROR
			};

			static void LogInfo(LogType type, const char* message);
			static void LogWarning(LogType type, const char* message);
			static void LogError(LogType type, const char* message);

			static void Assert(LogType type, bool condition, const char* message);
			
		private:
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