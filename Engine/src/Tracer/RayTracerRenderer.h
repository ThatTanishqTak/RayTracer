#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include <string>
#include <chrono>

#include <raylib.h>

namespace Engine
{
    class Scene; /// Forward declaration of the scene container.

    /**
     * \brief Describes a single measured step in the render pipeline.
     */
    struct RenderStep
    {
        std::string m_Name;       ///< Human readable name of the step.
        double m_ElapsedMs{ 0.0 };///< Time spent in this step in milliseconds.
    };

    /**
     * \brief Simple CPU based renderer that distributes tiles across worker threads.
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

        /**
         * \brief Obtain the fraction of tiles finished by worker threads.
         *
         * Values range from 0.0f when no work has been done to 1.0f once all
         * tiles have been processed.
         */
        float GetProgress() const;

        /**
         * \brief Access the list of recorded render steps.
         */
        const std::vector<RenderStep>& GetRenderSteps() const;

    private:
        /**
         * \brief Worker entry point. Each thread traces cache-friendly tiles.
         *
         * Tiles are fetched from a shared counter ensuring that each worker processes
         * unique image blocks without contention.
         */
        void WorkerThread(int threadID);

        std::vector<std::thread> m_Workers;              ///< Background workers performing the trace.
        std::atomic<bool> m_IsRendering{ false };        ///< True while workers are active.
        std::atomic<bool> m_StopRequested{ false };      ///< Signals workers to exit their loop.
        Image m_Framebuffer{};                           ///< CPU side color buffer.

        const Scene* m_CurrentScene{ nullptr };          ///< Scene being rendered.
        Camera m_CurrentCamera{};                        ///< Copy of camera for consistent rays.

        std::atomic<int> m_NextTile{ 0 };                ///< Index of the next tile to process.
        int m_TileSize{ 16 };                            ///< Width and height of a square tile.
        int m_TilesX{ 0 };                               ///< Number of tiles horizontally.
        int m_TilesY{ 0 };                               ///< Number of tiles vertically.
        int m_TotalTiles{ 0 };                           ///< Total amount of tiles in the frame.
        std::atomic<int> m_TilesCompleted{ 0 };          ///< Tiles completed by worker threads.

        std::vector<RenderStep> m_Steps;                 ///< Chronological steps recorded during rendering. TODO: make thread-safe.
        std::chrono::high_resolution_clock::time_point m_TileProcessingStart{}; ///< Start time for tile processing.
        std::atomic<int> m_WorkersFinished{ 0 };         ///< Number of workers that finished tracing.
    };
}