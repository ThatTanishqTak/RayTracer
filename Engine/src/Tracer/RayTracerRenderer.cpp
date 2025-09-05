#include "Tracer/RayTracerRenderer.h"

#include "Scene/Scene.h"
#include "Tracer/RayTracer.h"
#include "Tracer/Ray.h"

#include <raymath.h>
#include <algorithm>

namespace Engine
{
    void RayTracerRenderer::StartRender(const Scene& scene, const Camera& camera)
    {
        if (m_IsRendering)
        {
            // Avoid starting multiple render jobs simultaneously.
            return;
        }

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
                        static_cast<unsigned char>(Clamp(l_ColorVec.x, 0.0f, 1.0f) * 255.0f),
                        static_cast<unsigned char>(Clamp(l_ColorVec.y, 0.0f, 1.0f) * 255.0f),
                        static_cast<unsigned char>(Clamp(l_ColorVec.z, 0.0f, 1.0f) * 255.0f),
                        255
                    };

                    size_t l_Index = static_cast<size_t>(it_Y) * static_cast<size_t>(l_Width) + static_cast<size_t>(it_X);
                    reinterpret_cast<Color*>(m_Framebuffer.data)[l_Index] = l_Color;
                }
            }
        }

        // TODO: Add GPU compute path when available
    }
}