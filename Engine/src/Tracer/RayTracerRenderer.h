#pragma once

#include <vector>
#include <thread>
#include <atomic>

#include <raylib.h>

namespace Engine
{
    class Scene; /// Forward declaration of the scene container.

    /**
     * \brief Simple CPU based renderer that distributes scanlines across worker threads.
     *
     * The renderer owns a frame buffer image and spawns a pool of worker threads
     * to fill it with ray traced colors. Thread management is handled internally
     * and can be controlled via StartRender and StopRender.
     */
    class RayTracerRenderer
    {
    public:
        /**
         * \brief Begin rendering the provided scene from the given camera.
         *
         * If a render is already in progress this call has no effect.
         */
        void StartRender(const Scene& scene, const Camera& camera);

        /**
         * \brief Request all worker threads to stop and block until they finish.
         */
        void StopRender();

        /**
         * \brief Query whether the renderer currently has active worker threads.
         */
        bool IsRendering() const;

        /**
         * \brief Retrieve the current CPU-side frame buffer.
         */
        const Image& GetFrame() const;

    private:
        /**
         * \brief Worker entry point. Each thread traces its assigned scanlines.
         *
         * The scene is split by scanline where the thread ID determines the first
         * row and threads step by the total number of workers. This approach avoids
         * write contention because each thread writes to unique rows.
         */
        void WorkerThread(int threadID);

        std::vector<std::thread> m_Workers;          ///< Background workers performing the trace.
        std::atomic<bool> m_IsRendering{ false };    ///< True while workers are active.
        std::atomic<bool> m_StopRequested{ false };  ///< Signals workers to exit their loop.
        Image m_Framebuffer{};                       ///< CPU side color buffer.

        const Scene* m_CurrentScene{ nullptr };      ///< Scene being rendered.
        Camera m_CurrentCamera{};                    ///< Copy of camera for consistent rays.
    };
}