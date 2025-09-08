#include "Tracer/RayTracerRenderer.h"

#include "Scene/Scene.h"
#include "Tracer/RayTracer.h"
#include "Tracer/Ray.h"

#include <raymath.h>

#include <algorithm>
#include <chrono>

namespace Engine
{
    void RayTracerRenderer::StartRender(const Scene& scene, const Camera& camera)
    {
        if (m_IsRendering)
        {
            // Avoid starting multiple render jobs simultaneously.
            return;
        }

        // Reset previous step timings for a fresh render pass. Guarded to
        // prevent concurrent access while workers may still be finishing.
        {
            std::lock_guard<std::mutex> l_Lock(m_StepsMutex);
            m_Steps.clear();
        }

        std::chrono::high_resolution_clock::time_point l_SceneStart = std::chrono::high_resolution_clock::now();
        
        m_RenderStart = l_SceneStart; // Record absolute start of the render for duration queries.
        m_RenderDurationMs = 0.0;     // Reset previous duration.
        m_CurrentScene = &scene;
        m_CurrentCamera = camera; // Store a copy for consistent access across threads.
        m_StopRequested = false;
        m_IsRendering = true;

        // Allocate or resize the frame buffer to match the current window size.
        int l_Width = GetScreenWidth();
        int l_Height = GetScreenHeight();
        if (m_Framebuffer.data == nullptr || m_Framebuffer.width != l_Width || m_Framebuffer.height != l_Height)
        {
            if (m_Framebuffer.data != nullptr)
            {
                UnloadImage(m_Framebuffer);
            }
            m_Framebuffer = GenImageColor(l_Width, l_Height, BLACK);
        }

        // Prepare tile bookkeeping.
        m_TilesX = (l_Width + m_TileSize - 1) / m_TileSize;
        m_TilesY = (l_Height + m_TileSize - 1) / m_TileSize;
        m_TotalTiles = m_TilesX * m_TilesY;
        m_NextTile.store(0);
        m_TilesCompleted.store(0); // Reset progress counter at the start of a new render.

        // Spawn one worker per hardware thread.
        unsigned int l_ThreadCount = std::thread::hardware_concurrency();
        if (l_ThreadCount == 0)
        {
            l_ThreadCount = 1; // Fallback to single thread when unknown.
        }

        m_Workers.reserve(l_ThreadCount);
        for (unsigned int it_Thread = 0; it_Thread < l_ThreadCount; ++it_Thread)
        {
            // Each thread executes WorkerThread with its ID.
            m_Workers.emplace_back([this, it_Thread]() { WorkerThread(static_cast<int>(it_Thread)); });
        }

        std::chrono::high_resolution_clock::time_point l_SceneEnd = std::chrono::high_resolution_clock::now();
        double l_SceneMs = std::chrono::duration<double, std::milli>(l_SceneEnd - l_SceneStart).count();
        // Store the scene setup time so the UI can present it later.
        {
            std::lock_guard<std::mutex> l_Lock(m_StepsMutex);
            m_Steps.push_back(RenderStep{ "Scene Setup", l_SceneMs });
        }

        // Record start time for tile processing stage.
        m_TileProcessingStart = l_SceneEnd;
        m_WorkersFinished.store(0);
    }

    void RayTracerRenderer::StopRender()
    {
        if (!m_IsRendering)
        {
            return;
        }

        // Signal threads to stop and wait for them to exit.
        m_StopRequested = true;
        for (std::thread& it_Worker : m_Workers)
        {
            if (it_Worker.joinable())
            {
                it_Worker.join();
            }
        }

        m_Workers.clear();
        m_IsRendering = false;
    }

    bool RayTracerRenderer::IsRendering() const
    {
        return m_IsRendering.load();
    }

    const Image& RayTracerRenderer::GetFrame() const
    {
        return m_Framebuffer;
    }

    float RayTracerRenderer::GetProgress() const
    {
        // Avoid division by zero in cases where no render has been scheduled yet.
        if (m_TotalTiles == 0)
        {
            return 0.0f;
        }

        return m_TilesCompleted.load() / static_cast<float>(m_TotalTiles);
    }

    std::vector<RenderStep> RayTracerRenderer::GetRenderSteps() const
    {
        // Copy the step list under lock to avoid exposing mutable state.
        std::lock_guard<std::mutex> l_Lock(m_StepsMutex);

        return m_Steps;
    }

    void RayTracerRenderer::WorkerThread(int threadID)
    {
        (void)threadID; // Thread ID currently unused; reserved for future enhancements.
        int l_Width = m_Framebuffer.width;
        int l_Height = m_Framebuffer.height;

        while (!m_StopRequested)
        {
            int l_TileIndex = m_NextTile.fetch_add(1);
            if (l_TileIndex >= m_TotalTiles)
            {
                break;
            }

            int l_TileX = (l_TileIndex % m_TilesX) * m_TileSize;
            int l_TileY = (l_TileIndex / m_TilesX) * m_TileSize;
            int l_MaxX = std::min(l_TileX + m_TileSize, l_Width);
            int l_MaxY = std::min(l_TileY + m_TileSize, l_Height);

            for (int it_Y = l_TileY; it_Y < l_MaxY && !m_StopRequested; ++it_Y)
            {
                for (int it_X = l_TileX; it_X < l_MaxX; ++it_X)
                {
                    // Placeholder ray generation. A full implementation would derive the ray
                    // from the camera parameters and image plane coordinates using
                    // m_CurrentCamera and m_CurrentScene.
                    Ray l_Ray(Vector3{}, Vector3{ 0.0f, 0.0f, -1.0f });
                    Vector3 l_ColorVec = RayColor(l_Ray, m_CurrentScene->GetBVH(), 1);

                    Color l_Color
                    {
                        static_cast<unsigned char>(Clamp(l_ColorVec.x, 0.0f, 1.0f) * 255.0f),   // R
                        static_cast<unsigned char>(Clamp(l_ColorVec.y, 0.0f, 1.0f) * 255.0f),   // G
                        static_cast<unsigned char>(Clamp(l_ColorVec.z, 0.0f, 1.0f) * 255.0f),   // B
                        255                                                                     // A
                    };

                    size_t l_Index = static_cast<size_t>(it_Y) * static_cast<size_t>(l_Width) + static_cast<size_t>(it_X);
                    reinterpret_cast<Color*>(m_Framebuffer.data)[l_Index] = l_Color;
                }
            }

            // Atomically track how many tiles have been fully processed so far.
            m_TilesCompleted.fetch_add(1);
        }

        std::chrono::high_resolution_clock::time_point l_End = std::chrono::high_resolution_clock::now();
        if (m_WorkersFinished.fetch_add(1) + 1 == static_cast<int>(m_Workers.size()))
        {
            double l_ProcessMs = std::chrono::duration<double, std::milli>(l_End - m_TileProcessingStart).count();
            // Worker threads record the total tile processing time once all
            // threads finish. Protect the shared list with a mutex.
            {
                std::lock_guard<std::mutex> l_Lock(m_StepsMutex);
                m_Steps.push_back(RenderStep{ "Tile Processing", l_ProcessMs });
            }

            // Compute the overall duration including scene setup for UI queries.
            m_RenderDurationMs = std::chrono::duration<double, std::milli>(l_End - m_RenderStart).count();

            // Rendering is complete; reset flag so the UI knows no work is active.
            m_IsRendering = false;

            // Join all worker threads to reclaim resources. The current thread
            // cannot join itself, so join others first and detach this thread
            // before clearing the container.
            std::thread::id l_ThisThreadID = std::this_thread::get_id();
            for (std::thread& it_Worker : m_Workers)
            {
                if (it_Worker.get_id() != l_ThisThreadID && it_Worker.joinable())
                {
                    it_Worker.join();
                }
            }
            
            for (std::thread& it_Worker : m_Workers)
            {
                if (it_Worker.get_id() == l_ThisThreadID && it_Worker.joinable())
                {
                    it_Worker.detach();
                    break;
                }
            }

            m_Workers.clear();
        }

        // TODO: Add GPU compute path when available
    }
}