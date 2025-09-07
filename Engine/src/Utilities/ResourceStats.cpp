#include "ResourceStats.h"

#include <psapi.h>

namespace Engine
{
    namespace Utilities
    {
        // Initialize static members.
        ULONGLONG ResourceStats::s_LastIdleTime = 0;
        ULONGLONG ResourceStats::s_LastKernelTime = 0;
        ULONGLONG ResourceStats::s_LastUserTime = 0;

        size_t ResourceStats::GetMemoryUsage()
        {
            // Query memory statistics for the current process.
            PROCESS_MEMORY_COUNTERS l_Counters{};
            HANDLE l_Process = GetCurrentProcess();
            if (GetProcessMemoryInfo(l_Process, &l_Counters, sizeof(l_Counters)))
            {
                return static_cast<size_t>(l_Counters.WorkingSetSize);
            }

            return 0;
        }

        float ResourceStats::GetCPUUsage()
        {
            // Retrieve aggregate system times.
            FILETIME l_Idle{}, l_Kernel{}, l_User{};
            if (!GetSystemTimes(&l_Idle, &l_Kernel, &l_User))
            {
                return 0.0f;
            }

            // Convert FILETIME structures to 64-bit integers.
            ULONGLONG l_IdleTime = (static_cast<ULONGLONG>(l_Idle.dwHighDateTime) << 32) | l_Idle.dwLowDateTime;
            ULONGLONG l_KernelTime = (static_cast<ULONGLONG>(l_Kernel.dwHighDateTime) << 32) | l_Kernel.dwLowDateTime;
            ULONGLONG l_UserTime = (static_cast<ULONGLONG>(l_User.dwHighDateTime) << 32) | l_User.dwLowDateTime;

            // Calculate differences from the last measurement.
            ULONGLONG l_IdleDiff = l_IdleTime - s_LastIdleTime;
            ULONGLONG l_KernelDiff = l_KernelTime - s_LastKernelTime;
            ULONGLONG l_UserDiff = l_UserTime - s_LastUserTime;

            // Store current values for the next call.
            s_LastIdleTime = l_IdleTime;
            s_LastKernelTime = l_KernelTime;
            s_LastUserTime = l_UserTime;

            ULONGLONG l_Total = l_KernelDiff + l_UserDiff;
            if (l_Total == 0)
            {
                return 0.0f;
            }

            // Percentage of time spent executing non-idle threads.
            return static_cast<float>(l_Total - l_IdleDiff) * 100.0f / static_cast<float>(l_Total);
        }
    }
}