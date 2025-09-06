#pragma once

// This module provides simple helpers for retrieving resource usage for the
// current process using Win32 APIs.
// Memory usage is reported as the current working set size in bytes and
// CPU usage is estimated as the percentage of time the system spends
// executing non-idle threads since the last query.

#include <cstddef>
#include <windows.h>

namespace Engine
{
    namespace Utilities
    {
        /**
         * \brief Static helpers for querying basic process resource usage.
         */
        class ResourceStats
        {
        public:
            /**
             * \brief Obtain the current working set size for this process.
             * \return Memory usage in bytes or 0 on failure.
             */
            static size_t GetMemoryUsage();

            /**
             * \brief Compute system wide CPU usage since the previous call.
             * \return CPU usage percentage in the range [0, 100].
             */
            static float GetCPUUsage();

        private:
            // Store previous system times to calculate CPU usage deltas.
            static ULONGLONG s_LastIdleTime;    ///< Last observed idle time.
            static ULONGLONG s_LastKernelTime; ///< Last observed kernel time.
            static ULONGLONG s_LastUserTime;   ///< Last observed user time.
        };
    }
}
