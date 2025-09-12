#include "ResourceStats.h"

#include <psapi.h>

#ifdef ENGINE_ENABLE_GPU
#include <GL/glew.h>
#include <string_view>
#endif

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

        size_t ResourceStats::GetGPUMemoryUsage()
        {
#ifdef ENGINE_ENABLE_GPU
            // Cache extension checks so expensive queries happen only once.
            static bool s_Checked = false;
            static bool s_HasNVX = false; // NVIDIA specific extension presence
            static bool s_HasATI = false; // AMD specific extension presence

            if (!s_Checked)
            {
                const GLubyte* l_ExtensionsBytes = glGetString(GL_EXTENSIONS);
                if (l_ExtensionsBytes)
                {
                    std::string_view l_Extensions = reinterpret_cast<const char*>(l_ExtensionsBytes);
                    s_HasNVX = l_Extensions.find("GL_NVX_gpu_memory_info") != std::string_view::npos;
                    s_HasATI = l_Extensions.find("GL_ATI_meminfo") != std::string_view::npos;
                }
                s_Checked = true;
            }

            if (s_HasNVX)
            {
                // NVIDIA path: reports dedicated and available memory in kilobytes.
                GLint l_DedicatedKB = 0;
                GLint l_AvailableKB = 0;
                glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &l_DedicatedKB);
                glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &l_AvailableKB);
                GLint l_UsedKB = l_DedicatedKB - l_AvailableKB;
                return static_cast<size_t>(l_UsedKB) * 1024; // Convert to bytes
            }

            if (s_HasATI)
            {
                // AMD path: extension exposes only free memory in kilobytes.
                // Cache total on first call and compute usage as difference.
                static GLint s_TotalMemoryKB = 0;
                GLint l_MemoryKB[4] = {};
                glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, l_MemoryKB);
                if (s_TotalMemoryKB == 0)
                {
                    s_TotalMemoryKB = l_MemoryKB[0];
                    return 0; // First call can't determine usage yet
                }
                GLint l_UsedKB = s_TotalMemoryKB - l_MemoryKB[0];
                return static_cast<size_t>(l_UsedKB) * 1024; // Convert to bytes
            }
#endif
            // Unsupported GPU query or no GPU build.
            return 0;
        }
    }
}