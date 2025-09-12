#include "Renderer/Renderer.h"
#include "Utilities/Logging.h"

#include "Scene/Scene.h"
#include "Tracer/Ray.h"
#include "Tracer/RayTracer.h"
#include "Tracer/GPUPrimitives.h"

// Include GLEW before raylib to ensure GLsync and sync enums are available.
// Requires OpenGL 3.2+ for synchronization objects.
#include <GL/glew.h>
#include <raylib.h>
#include <raymath.h>
#include <rlImGui.h>
#ifdef ENGINE_GPU_COMPUTE_AVAILABLE
#include <rlgl.h>
#endif

// Future improvement: transition to cross-platform APIs like Vulkan or DirectX.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <format>
#include <random>
#include <string>

namespace
{
    // Thread-local RNG avoids contention when jittering sample rays per worker.
    thread_local std::mt19937 s_Generator(std::random_device{}());
    thread_local std::uniform_real_distribution<float> s_Distribution(0.0f, 1.0f);
}

namespace Engine
{
    bool Renderer::Initialize(int width, int height)
    {
        RAY_CORE_TRACE("Initializing the renderer");

        bool l_Result = true; // Track overall initialization success

        // Load OpenGL function pointers. Must occur after the context is created.
        if (l_Result)
        {
            GLenum l_GlewError = glewInit();
            if (l_GlewError != GLEW_OK)
            {
                RAY_CORE_ERROR("Failed to initialize GLEW: {}", reinterpret_cast<const char*>(glewGetErrorString(l_GlewError)));

                l_Result = false;
            }
        }

#if !defined(ENGINE_GPU_COMPUTE_AVAILABLE)
        // Ensure CPU path is active when the engine or raylib lacks compute support.
        SetRenderMode(RenderMode::RayTrace);

        // Query GPU information to aid users in diagnosing missing features.
        const GLubyte* l_VendorBytes = glGetString(GL_VENDOR);
        const GLubyte* l_RendererBytes = glGetString(GL_RENDERER);
        const GLubyte* l_DriverBytes = glGetString(GL_VERSION);
        const char* l_Vendor = l_VendorBytes ? reinterpret_cast<const char*>(l_VendorBytes) : "Unknown";
        const char* l_Renderer = l_RendererBytes ? reinterpret_cast<const char*>(l_RendererBytes) : "Unknown";
        const char* l_Driver = l_DriverBytes ? reinterpret_cast<const char*>(l_DriverBytes) : "Unknown";

        RAY_CORE_ERROR("{} | GPU Vendor: {} | GPU Renderer: {} | Driver Version: {}. Consider updating GPU drivers or enabling compute support in configuration.",
            "GPU compute path unavailable; defaulting to CPU ray tracing", l_Vendor, l_Renderer, l_Driver);
#endif

        // Initialize ImGui for raylib and store the result.
        rlImGuiSetup(true);
        bool l_RlImGuiInitialized = true;
        if (!l_RlImGuiInitialized)
        {
            // Log failure and abort initialization if ImGui setup fails.
            RAY_CORE_ERROR("Failed to initialize rlImGui");
            l_Result = false;

            return l_Result;
        }

        // Allocate a render texture matching the current window size.
        if (l_Result)
        {
            l_Result = ResizeFrameTexture(width, height);
        }

#ifdef ENGINE_GPU_COMPUTE_AVAILABLE
        // Load the compute shader used for GPU ray tracing.
        if (l_Result)
        {
            // Compute shaders require OpenGL 4.3 or equivalent. If the platform
            // lacks the required support, fall back to the CPU path and warn the
            // user. This keeps initialization successful while disabling GPU
            // tracing.
            if (rlGetVersion() < RL_OPENGL_43)
            {
                // Gather GPU details to help users diagnose unsupported compute capabilities.
                const GLubyte* l_VendorBytes = glGetString(GL_VENDOR);
                const GLubyte* l_RendererBytes = glGetString(GL_RENDERER);
                const GLubyte* l_DriverBytes = glGetString(GL_VERSION);
                const char* l_Vendor = l_VendorBytes ? reinterpret_cast<const char*>(l_VendorBytes) : "Unknown";
                const char* l_Renderer = l_RendererBytes ? reinterpret_cast<const char*>(l_RendererBytes) : "Unknown";
                const char* l_Driver = l_DriverBytes ? reinterpret_cast<const char*>(l_DriverBytes) : "Unknown";

                RAY_CORE_WARNING("Compute shaders unsupported; defaulting to CPU ray tracing | GPU Vendor: {} | GPU Renderer: {} | Driver Version: {}. Consider updating GPU drivers or verifying hardware support."
                    ,l_Vendor, l_Renderer, l_Driver);

                SetRenderMode(RenderMode::RayTrace);
            }

            else
            {
                // Resolve the shader relative to the executable location.
                // This avoids manual directory walking and prepares for a future
                // resource manager to handle asset lookups in a central place.
                std::filesystem::path l_ShaderPath = std::filesystem::path(GetApplicationDirectory()) / "Assets/Shaders/RayTrace.hlsl";
                if (!std::filesystem::exists(l_ShaderPath))
                {
                    // Fail early if the shader cannot be found to provide a clear message.
                    RAY_CORE_ERROR("Compute shader not found: {}", l_ShaderPath.string());

                    l_Result = false;
                }

                else
                {
                    std::string l_ShaderPathStr = l_ShaderPath.string();

                    // Load and compile the compute shader. LoadShader cannot be
                    // used because it only handles vertex/fragment pipelines, so
                    // the compute-specific rlLoadComputeShaderProgram is
                    // required here.
                    char* l_ShaderCode = LoadFileText(l_ShaderPathStr.c_str());
                    if (l_ShaderCode == nullptr)
                    {
                        RAY_CORE_ERROR("Failed to read compute shader: {}", l_ShaderPathStr);

                        l_Result = false;
                    }

                    else
                    {
                        unsigned int l_ShaderId = rlCompileShader(l_ShaderCode, RL_COMPUTE_SHADER);
                        UnloadFileText(l_ShaderCode);

                        if (l_ShaderId == 0)
                        {
                            RAY_CORE_ERROR("Failed to compile compute shader: {}", l_ShaderPathStr);

                            l_Result = false;
                        }

                        else
                        {
                            unsigned int l_ProgramId = rlLoadComputeShaderProgram(l_ShaderId);
                            if (l_ProgramId == 0)
                            {
                                RAY_CORE_ERROR("Failed to link compute shader program: {}", l_ShaderPathStr);

                                l_Result = false;
                            }

                            else
                            {
                                m_GpuPipeline.id = l_ProgramId;
                                m_GpuPipeline.locs = nullptr;
                            }
                        }
                    }
                }
            }
        }
#endif

        // Future improvement: provide detailed error codes instead of a simple
        // boolean.

        RAY_CORE_TRACE("Renderer initialized");

        return l_Result;
    }

    void Renderer::Shutdown()
    {
        RAY_CORE_TRACE("Shuting down renderer");

        // Release frame texture if it exists.
        if (m_RenderTexture.id != 0)
        {
            UnloadRenderTexture(m_RenderTexture);
        }

#ifdef ENGINE_GPU_COMPUTE_AVAILABLE
        // Release GPU compute resources when previously allocated.
        if (m_GpuPipeline.id != 0)
        {
            // Use compute-specific unload to match rlLoadComputeShaderProgram.
            rlUnloadShaderProgram(m_GpuPipeline.id);
            m_GpuPipeline = { 0 };
        }

        if (m_NodeBuffer != 0)
        {
            rlUnloadShaderBuffer(m_NodeBuffer);
            m_NodeBuffer = 0;
        }

        if (m_PrimitiveBuffer != 0)
        {
            rlUnloadShaderBuffer(m_PrimitiveBuffer);
            m_PrimitiveBuffer = 0;
        }

        if (m_MaterialBuffer != 0)
        {
            rlUnloadShaderBuffer(m_MaterialBuffer);
            m_MaterialBuffer = 0;
        }
#endif

        RAY_CORE_TRACE("Renderer shutdown complete");
    }

    void Renderer::BeginFrame()
    {
        // Prepare the screen for drawing.
        BeginDrawing();
        ClearBackground(BLACK);
    }

    void Renderer::EndFrame()
    {
        // Present the rendered frame to the display.
        EndDrawing();
    }

    void Renderer::Begin3D(Camera3D camera)
    {
        // Switch to 3D rendering mode.
        BeginMode3D(camera);
    }

    void Renderer::End3D()
    {
        // Return to 2D rendering mode.
        EndMode3D();
    }

    void Renderer::UpdateCamera(float deltaTime)
    {
        // Delegate input handling to the camera controller.
        // Centralizing camera logic simplifies future extensions such as custom
        // bindings or editor modes.
        m_CameraController.Update(deltaTime);
    }

    bool Renderer::ResizeFrameTexture(int width, int height)
    {
        // Avoid reallocating the texture if dimensions are unchanged.
        if (width == m_FrameWidth && height == m_FrameHeight)
        {
            return true;
        }

        // Release previous render texture if it exists.
        if (m_RenderTexture.id != 0)
        {
            UnloadRenderTexture(m_RenderTexture);
            m_RenderTexture = { 0 };
        }

        // Create a new render texture; this enables potential direct drawing without
        // CPU-GPU copy.
        RenderTexture2D l_NewTexture = LoadRenderTexture(width, height);
        if (l_NewTexture.id == 0)
        {
            // Preformat message to leverage C++20 formatting and avoid macro-specific
            // format strings
            std::string l_Message = std::format("Failed to allocate render texture {} x {}", width, height);
            RAY_CORE_ERROR(l_Message);

            m_FrameWidth = 0;
            m_FrameHeight = 0;
            m_CachedPixels.clear();

            return false;
        }

        m_RenderTexture = l_NewTexture;
        m_FrameWidth = width;
        m_FrameHeight = height;

        // Ensure cached pixel buffer matches new dimensions.
        m_CachedPixels.assign(static_cast<size_t>(width) * static_cast<size_t>(height), Color{ 0, 0, 0, 255 });

        // Future improvement: handle different pixel formats and color spaces.
        return true;
    }

    void Renderer::RenderImage(const Color* buffer, int width, int height)
    {
        // Ensure texture matches incoming buffer dimensions.
        if (!ResizeFrameTexture(width, height))
        {
            return; // Abort rendering if texture allocation failed
        }

        // Determine the region of pixels that changed since the last upload.
        bool l_Changed = false;
        int l_MinX = width;
        int l_MinY = height;
        int l_MaxX = 0;
        int l_MaxY = 0;

        for (int it_Y = 0; it_Y < height; ++it_Y)
        {
            for (int it_X = 0; it_X < width; ++it_X)
            {
                int l_Index = it_Y * width + it_X;
                const Color& l_NewColor = buffer[l_Index];
                Color& l_OldColor = m_CachedPixels[l_Index];

                if (l_NewColor.r != l_OldColor.r || l_NewColor.g != l_OldColor.g || l_NewColor.b != l_OldColor.b || l_NewColor.a != l_OldColor.a)
                {
                    l_OldColor = l_NewColor;
                    l_Changed = true;
                    if (it_X < l_MinX)
                    {
                        l_MinX = it_X;
                    }

                    if (it_Y < l_MinY)
                    {
                        l_MinY = it_Y;
                    }

                    if (it_X > l_MaxX)
                    {
                        l_MaxX = it_X;
                    }

                    if (it_Y > l_MaxY)
                    {
                        l_MaxY = it_Y;
                    }
                }
            }
        }

        // Nothing to update.
        if (!l_Changed)
        {
            return;
        }

        int l_UpdateWidth = l_MaxX - l_MinX + 1;
        int l_UpdateHeight = l_MaxY - l_MinY + 1;

        if (l_UpdateWidth == width && l_UpdateHeight == height)
        {
            // Entire texture changed.
            UpdateTexture(m_RenderTexture.texture, m_CachedPixels.data());

            return;
        }

        // Copy changed region into a persistent sub-buffer to minimize allocations.
        size_t l_SubBufferSize = static_cast<size_t>(l_UpdateWidth) * static_cast<size_t>(l_UpdateHeight);
        if (m_SubBuffer.size() != l_SubBufferSize)
        {
            // Future improvement: reserve capacity in larger chunks or use tiling for
            // massive images.
            m_SubBuffer.resize(l_SubBufferSize);
        }

        for (int it_Y = 0; it_Y < l_UpdateHeight; ++it_Y)
        {
            Color* l_Dst = &m_SubBuffer[static_cast<size_t>(it_Y) * static_cast<size_t>(l_UpdateWidth)];
            const Color* l_Src = &m_CachedPixels[(l_MinY + it_Y) * width + l_MinX];
            
            std::copy(l_Src, l_Src + l_UpdateWidth, l_Dst);
        }

        Rectangle l_Rect{ static_cast<float>(l_MinX), static_cast<float>(l_MinY), static_cast<float>(l_UpdateWidth), static_cast<float>(l_UpdateHeight) };

        UpdateTextureRec(m_RenderTexture.texture, l_Rect, m_SubBuffer.data());

        // Future improvement: implement tiled texture updates to further reduce
        // CPU-GPU transfers.
    }

    const Camera& Renderer::GetCamera() const
    {
        return m_CameraController.GetCamera();
    }

    const Texture2D& Renderer::GetFrameTexture() const
    {
        return m_RenderTexture.texture;
    }

    int Renderer::GetTileSize() const
    {
        // Current tile side length in pixels.
        // Future improvement: expose runtime configuration or adaptive sizing.
        return m_TileSize;
    }

    void Renderer::SetRenderMode(RenderMode mode)
    {
        // Switch between available rendering pipelines.
        switch (mode)
        {
        case RenderMode::Raster:
        case RenderMode::RayTrace:
            m_RenderMode = mode;
            break;
        case RenderMode::RayTraceGPU:
#ifdef ENGINE_GPU_COMPUTE_AVAILABLE
            m_RenderMode = mode;
#else
            // Fallback to CPU tracing if GPU support is not compiled in.
            m_RenderMode = RenderMode::RayTrace;
#endif
            break;
        default:
            m_RenderMode = RenderMode::Raster;
            break;
        }

        // GPU mode is ideal for complex scenes or high sample counts when hardware
        // acceleration is available.
    }

    Renderer::RenderMode Renderer::GetRenderMode() const
    {
        // Report current rendering technique. GPU mode requires ENGINE_GPU_COMPUTE_AVAILABLE at
        // build time.
        return m_RenderMode;
    }

    void Renderer::StartRender(const Scene& scene, const Camera& camera)
    {
        if (m_IsRendering)
        {
            // Avoid starting multiple render jobs simultaneously.
            return;
        }

        // Reset previous step timings for a fresh render pass.
        {
            std::lock_guard<std::mutex> l_Lock(m_StepsMutex);
            m_Steps.clear();
        }

        std::chrono::high_resolution_clock::time_point l_SceneStart = std::chrono::high_resolution_clock::now();

        // Record absolute start of the render for duration queries.
        m_RenderStart = l_SceneStart;
        m_RenderDurationMs = 0.0; // Reset previous duration.
        m_CurrentScene = &scene;
        m_CurrentCamera = camera; // Store a copy for consistent access across threads.
        m_StopRequested = false;
        m_IsRendering = true;
        m_GpuDispatched = false; // Reset GPU dispatch flag for this render pass

#ifdef ENGINE_GPU_COMPUTE_AVAILABLE
        if (m_GpuFence != nullptr)
        {
            // Release leftover fence from a previous run.
            glDeleteSync(m_GpuFence);
            m_GpuFence = nullptr;
        }
        m_GpuCompleted = false;          // No GPU result yet
        m_GpuCompletionNotified = false; // UI has not been informed of completion
#endif

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

        // Handle GPU dispatch separately to bypass worker threads and atomics.
        if (m_RenderMode == RenderMode::RayTraceGPU)
        {
#ifdef ENGINE_GPU_COMPUTE_AVAILABLE
            std::chrono::high_resolution_clock::time_point l_SceneEnd = std::chrono::high_resolution_clock::now();
            double l_SceneMs = std::chrono::duration<double, std::milli>(l_SceneEnd - l_SceneStart).count();
            {
                std::lock_guard<std::mutex> l_Lock(m_StepsMutex);
                m_Steps.push_back(RenderStep{ "Scene Setup", l_SceneMs });
            }

            std::chrono::high_resolution_clock::time_point l_DispatchStart = l_SceneEnd;
            DispatchGPU(scene, camera);
            m_GpuDispatched = true; // Mark that the GPU path executed
            // Insert a fence so the CPU can later query when the GPU work is finished.
            m_GpuFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            glFlush(); // Ensure the GPU begins processing the queued work
            std::chrono::high_resolution_clock::time_point l_DispatchEnd = std::chrono::high_resolution_clock::now();
            double l_DispatchMs = std::chrono::duration<double, std::milli>(l_DispatchEnd - l_DispatchStart).count();
            {
                std::lock_guard<std::mutex> l_Lock(m_StepsMutex);
                m_Steps.push_back(RenderStep{ "GPU Dispatch", l_DispatchMs });
            }
            m_RenderDurationMs = std::chrono::duration<double, std::milli>(l_DispatchEnd - m_RenderStart).count();
#endif
            // GPU work runs asynchronously; keep m_IsRendering true until the fence signals.

            return;
        }

        // Prepare tile bookkeeping for CPU path.
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
            m_Workers.emplace_back([this, it_Thread]()
                {
                    WorkerThread(static_cast<int>(it_Thread));
                });
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

    void Renderer::StopRender()
    {
        if (!m_IsRendering)
        {
            return;
        }

        if (m_RenderMode == RenderMode::RayTraceGPU)
        {
#ifdef ENGINE_GPU_COMPUTE_AVAILABLE
            if (m_GpuFence != nullptr)
            {
                // Wait for any outstanding GPU work. OpenGL lacks explicit cancellation,
                // so the CPU blocks here until the fence signals completion.
                // Future improvement: attempt true cancellation on APIs that support it.
                glClientWaitSync(m_GpuFence, GL_SYNC_FLUSH_COMMANDS_BIT, UINT64_MAX);
                glDeleteSync(m_GpuFence);
                m_GpuFence = nullptr;
            }

            if (!m_GpuCompleted)
            {
                // Retrieve the texture contents so GetFrame() has valid data even when
                // StopRender() is invoked before GetProgress() observes completion.
                if (m_Framebuffer.data != nullptr)
                {
                    UnloadImage(m_Framebuffer);
                }
                m_Framebuffer = LoadImageFromTexture(m_RenderTexture.texture);
                m_GpuCompleted = true;
                m_GpuCompletionNotified = false; // Allow UI to observe completion once
            }
#endif
            m_IsRendering = false;

            return;
        }

        m_StopRequested = true;
        for (std::jthread& it_Worker : m_Workers)
        {
            if (it_Worker.joinable())
            {
                it_Worker.join();
            }
        }

        m_Workers.clear();
        m_IsRendering = false;
    }

    bool Renderer::IsRendering() const
    {
        return m_IsRendering.load();
    }

    const Image& Renderer::GetFrame() const
    {
        return m_Framebuffer;
    }

    float Renderer::GetProgress()
    {
        if (m_RenderMode == RenderMode::RayTraceGPU)
        {
#ifdef ENGINE_GPU_COMPUTE_AVAILABLE
            if (m_GpuFence != nullptr && !m_GpuCompleted)
            {
                // Poll the fence with zero timeout so the CPU never stalls.
                GLenum l_Result = glClientWaitSync(m_GpuFence, 0, 0);
                if (l_Result == GL_ALREADY_SIGNALED || l_Result == GL_CONDITION_SATISFIED)
                {
                    glDeleteSync(m_GpuFence);
                    m_GpuFence = nullptr;
                    // Read back the texture so GetFrame() exposes the GPU result.
                    // Future improvement: perform this read-back asynchronously to
                    // avoid stalling the main thread on large frames.
                    if (m_Framebuffer.data != nullptr)
                    {
                        UnloadImage(m_Framebuffer);
                    }
                    m_Framebuffer = LoadImageFromTexture(m_RenderTexture.texture);
                    m_GpuCompleted = true;
                    m_GpuCompletionNotified = false; // Ensure one more frame renders the result
                }
            }

            if (m_GpuCompleted)
            {
                if (!m_GpuCompletionNotified)
                {
                    // Report completion once while keeping m_IsRendering true so the caller
                    // can blit the final frame this frame.
                    m_GpuCompletionNotified = true;
                    return 1.0f;
                }

                if (m_IsRendering)
                {
                    // On the following frame flip the flag so rendering code knows GPU work ended.
                    m_IsRendering = false;
                }

                return 1.0f;
            }

            if (!m_GpuDispatched)
            {
                // No dispatch occurred; report zero progress for clarity.
                return 0.0f;
            }

            return 0.0f; // Dispatch in flight
#else
            (void)m_GpuDispatched; // Avoid unused member warning in non-GPU builds
            return 0.0f;
#endif
        }
    }

    std::vector<RenderStep> Renderer::GetRenderSteps() const
    {
        // Return a thread-safe copy of the recorded render steps.
        std::scoped_lock l_Lock(m_StepsMutex);
        std::vector<RenderStep> l_Copy = m_Steps;

        // Future improvement: expose a non-owning view to avoid the copy overhead.
        return l_Copy;
    }

#ifdef ENGINE_GPU_COMPUTE_AVAILABLE
    void Renderer::DispatchGPU(const Scene& scene, const Camera& camera)
    {
        if (m_GpuPipeline.id == 0)
        {
            // Future improvement: lazily create the compute pipeline here.
            return;
        }

        // Upload flattened BVH nodes and primitives to shader storage buffers.
        const std::vector<BVHFlatNode>& l_Nodes = scene.GetFlatNodes();
        const std::vector<SphereGPU>& l_Primitives = scene.GetFlatPrimitives();
        const std::vector<MaterialGPU>& l_Materials = scene.GetFlatMaterials();

        if (!l_Nodes.empty())
        {
            unsigned int l_Required = static_cast<unsigned int>(l_Nodes.size() * sizeof(BVHFlatNode));
            if (m_NodeBuffer == 0 || rlGetShaderBufferSize(m_NodeBuffer) < l_Required)
            {
                if (m_NodeBuffer != 0)
                {
                    rlUnloadShaderBuffer(m_NodeBuffer);
                }
                m_NodeBuffer = rlLoadShaderBuffer(l_Required, l_Nodes.data(), RL_STATIC_DRAW);
            }

            else
            {
                rlUpdateShaderBuffer(m_NodeBuffer, l_Nodes.data(), l_Required, 0);
            }
            rlBindShaderBuffer(m_NodeBuffer, 0);
        }

        if (!l_Primitives.empty())
        {
            unsigned int l_Required = static_cast<unsigned int>(l_Primitives.size() * sizeof(SphereGPU));
            if (m_PrimitiveBuffer == 0 || rlGetShaderBufferSize(m_PrimitiveBuffer) < l_Required)
            {
                if (m_PrimitiveBuffer != 0)
                {
                    rlUnloadShaderBuffer(m_PrimitiveBuffer);
                }
                m_PrimitiveBuffer = rlLoadShaderBuffer(l_Required, l_Primitives.data(), RL_STATIC_DRAW);
            }

            else
            {
                rlUpdateShaderBuffer(m_PrimitiveBuffer, l_Primitives.data(), l_Required, 0);
            }
            rlBindShaderBuffer(m_PrimitiveBuffer, 1);
        }

        if (!l_Materials.empty())
        {
            unsigned int l_Required = static_cast<unsigned int>(l_Materials.size() * sizeof(MaterialGPU));
            if (m_MaterialBuffer == 0 || rlGetShaderBufferSize(m_MaterialBuffer) < l_Required)
            {
                if (m_MaterialBuffer != 0)
                {
                    rlUnloadShaderBuffer(m_MaterialBuffer);
                }
                m_MaterialBuffer = rlLoadShaderBuffer(l_Required, l_Materials.data(), RL_STATIC_DRAW);
            }

            else
            {
                rlUpdateShaderBuffer(m_MaterialBuffer, l_Materials.data(), l_Required, 0);
            }
            rlBindShaderBuffer(m_MaterialBuffer, 2);
        }

        // Upload basic per-frame parameters as uniform values.
        Vector3 l_CameraPos = camera.position;
        int l_Location = GetShaderLocation(m_GpuPipeline, "uCameraPos");
        SetShaderValue(m_GpuPipeline, l_Location, &l_CameraPos, SHADER_UNIFORM_VEC3);

        struct TraceParams
        {
            int m_MaxDepth;
            int m_SamplesPerPixel;
        };

        TraceParams l_Params{ m_MaxDepth, m_SamplesPerPixel };
        int l_ParamsLoc = GetShaderLocation(m_GpuPipeline, "uParams");
        SetShaderValue(m_GpuPipeline, l_ParamsLoc, &l_Params, SHADER_UNIFORM_IVEC2);

        // Bind the render texture as an image (UAV) so the compute shader can write
        // the final color directly. This avoids an intermediate CPU copy and keeps
        // the entire ray tracing pass on the GPU.
        rlBindImageTexture(m_RenderTexture.texture.id, 0, m_RenderTexture.texture.format, false);

        // Launch one work group per 8x8 tile of the framebuffer. The work group
        // size is chosen to match the layout expected by the compute shader.
        //rlDispatchCompute(m_FrameWidth / 8, m_FrameHeight / 8, 1);

        // Ensure all writes to the image are complete before the texture is used
        // for presentation. Without this barrier some GPUs could display partially
        // updated data.
        //rlMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // The result now lives in m_RenderTexture and can be presented directly.
        // Future improvement: read back into m_Framebuffer for CPU-side effects or
        // dispatch the compute shader asynchronously and in smaller tiles.
    }
#endif

    void Renderer::WorkerThread(int threadID)
    {
        (void)threadID; // Thread ID currently unused; reserved for future enhancements.

        int l_Width = m_Framebuffer.width;
        int l_Height = m_Framebuffer.height;
        std::mt19937& l_Generator = s_Generator; // Reuse thread-local RNG for jitter

        // Precompute camera basis vectors and projection parameters once per thread.
        // These form an orthonormal basis from the camera orientation and scale the
        // normalized device coordinates to the correct field of view. The math here
        // lays the groundwork for future features like depth of field or motion
        // blur by establishing a precise mapping from pixel to world-space ray.
        Vector3 l_Forward = Vector3Normalize(Vector3Subtract(m_CurrentCamera.target, m_CurrentCamera.position));
        Vector3 l_Right = Vector3Normalize(Vector3CrossProduct(l_Forward, m_CurrentCamera.up));
        Vector3 l_Up = Vector3CrossProduct(l_Right, l_Forward);

        float l_FovY = m_CurrentCamera.fovy * DEG2RAD;
        float l_TanFovY = tanf(l_FovY * 0.5f);
        float l_TanFovX = l_TanFovY * static_cast<float>(l_Width) / static_cast<float>(l_Height);

        while (!m_StopRequested)
        {
            // TODO: Explore GPU compute for parallel tile processing.
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
                    // Accumulate multiple jittered samples per pixel to reduce aliasing.
                    // This increases render time linearly with the sample count.
                    // Future improvement: incorporate stratified sampling for better
                    // convergence.
                    Vector3 l_ColorAccum{ 0.0f, 0.0f, 0.0f };
                    for (int it_Sample = 0; it_Sample < m_SamplesPerPixel; ++it_Sample)
                    {
                        float l_OffsetX = s_Distribution(l_Generator);
                        float l_OffsetY = s_Distribution(l_Generator);

                        // Convert pixel coordinates plus jitter to NDC in [-1,1].
                        float l_NdcX = ((static_cast<float>(it_X) + l_OffsetX) / static_cast<float>(l_Width)) * 2.0f - 1.0f;
                        float l_NdcY = 1.0f - ((static_cast<float>(it_Y) + l_OffsetY) / static_cast<float>(l_Height)) * 2.0f;

                        // Ray direction through the pixel on the virtual image plane.
                        Vector3 l_PixelDir = Vector3Normalize(Vector3Add(Vector3Add(l_Forward, Vector3Scale(l_Right, l_NdcX * l_TanFovX)), Vector3Scale(l_Up, l_NdcY * l_TanFovY)));

                        Ray l_Ray(m_CurrentCamera.position, l_PixelDir);
                        // Trace the ray using the configured maximum depth to limit
                        // recursion. Future improvement: expose m_MaxDepth for runtime
                        // tuning.
                        Vector3 l_SampleColor = RayColor(l_Ray, m_CurrentScene->GetBVH(), m_MaxDepth);
                        l_ColorAccum = Vector3Add(l_ColorAccum, l_SampleColor);
                    }

                    Vector3 l_ColorVec = Vector3Scale(l_ColorAccum, 1.0f / static_cast<float>(m_SamplesPerPixel));

                    // Apply gamma correction (gamma = 2.0) to approximate how monitors
                    // display brightness. This converts the linear color returned by the
                    // ray tracer to a non-linear representation that better matches human
                    // perception. Future improvement: make the gamma value configurable for
                    // different output devices or to disable correction entirely.
                    Vector3 l_CorrectedColor{ sqrtf(l_ColorVec.x), sqrtf(l_ColorVec.y), sqrtf(l_ColorVec.z) };

                    // Clamp the gamma-corrected color and convert to 8-bit per channel.
                    Color l_Color
                    {
                        static_cast<unsigned char>(Clamp(l_CorrectedColor.x, 0.0f, 1.0f) * 255.0f), // R
                        static_cast<unsigned char>(Clamp(l_CorrectedColor.y, 0.0f, 1.0f) * 255.0f), // G
                        static_cast<unsigned char>(Clamp(l_CorrectedColor.z, 0.0f, 1.0f) * 255.0f), // B
                        255                                                                         // A
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
            // Worker threads record the total tile processing time once all threads
            // finish.
            {
                std::lock_guard<std::mutex> l_Lock(m_StepsMutex);
                m_Steps.push_back(RenderStep{ "Tile Processing", l_ProcessMs });
            }

            // Compute the overall duration including scene setup for UI queries.
            m_RenderDurationMs = std::chrono::duration<double, std::milli>(l_End - m_RenderStart).count();

            // Rendering is complete; reset flag so the UI knows no work is active.
            m_IsRendering = false;

            // Join all worker threads to reclaim resources. The current thread cannot
            // join itself, so join others first and detach this thread before clearing
            // the container.
            std::thread::id l_ThisThreadID = std::this_thread::get_id();
            for (std::jthread& it_Worker : m_Workers)
            {
                if (it_Worker.get_id() != l_ThisThreadID && it_Worker.joinable())
                {
                    it_Worker.join();
                }
            }

            for (std::jthread& it_Worker : m_Workers)
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