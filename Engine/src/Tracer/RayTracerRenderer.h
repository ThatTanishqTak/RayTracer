#pragma once

#include "Renderer/Renderer.h"
#include <vector>
#include <raylib.h>

namespace Engine
{
    class Scene; /// Forward declaration of the scene container.

    /**
     * \brief Simple CPU based renderer that distributes tiles across worker threads.
     *
     * The renderer owns a frame buffer image and spawns a pool of worker threads
     * to fill it with ray traced colors. Thread management is handled internally
     * and can be controlled via StartRender and StopRender.
     */
    class RayTracerRenderer : public Renderer
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

        /**
         * \brief Obtain the fraction of tiles finished by worker threads.
         *
         * Values range from 0.0f when no work has been done to 1.0f once all
         * tiles have been processed.
         */
        float GetProgress() const;

        /**
         * \brief Retrieve a thread-safe copy of the recorded render steps.
         *
         * The internal step list is protected by a mutex to prevent data races
         * when worker threads record their timing information. This method
         * acquires the mutex and returns a snapshot of the data for the caller
         * to inspect without holding the lock.
         */
        std::vector<RenderStep> GetRenderSteps() const;

    private:
        /**
         * \brief Worker entry point. Each thread traces cache-friendly tiles.
         *
         * Tiles are fetched from a shared counter ensuring that each worker processes
         * unique image blocks without contention.
         */
        void WorkerThread(int threadID);
    };
}