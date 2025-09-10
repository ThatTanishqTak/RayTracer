#include "SandboxLayer.h"

#ifdef _WIN32
// Avoid collisions between Raylib symbols and Win32 macros.
#define CloseWindow RaylibCloseWindow // Preserve Raylib's CloseWindow function
#define ShowCursor RaylibShowCursor   // Preserve Raylib's ShowCursor function
#define Rectangle RaylibRectangle     // Preserve Raylib's Rectangle type
#define LoadImage RaylibLoadImage     // Preserve Raylib's LoadImage function
#define DrawText RaylibDrawText       // Preserve Raylib's DrawText function
#define DrawTextEx RaylibDrawTextEx   // Preserve Raylib's DrawTextEx function
#include <Windows.h>                  // Win32 API header
#undef CloseWindow                    // Restore original Raylib names
#undef ShowCursor
#undef Rectangle
#undef LoadImage
#undef DrawText
#undef DrawTextEx
#endif

#include "Utilities/ResourceStats.h"
#include <Renderer/Material.h>

#include <imgui.h>
#include <rlImGui.h>

#include <cfloat>
#include <memory>

// The layer drives the CPU ray tracer through the generic Renderer and uses
// the application's display renderer to present the results. This simplified
// flow removes the legacy RayTracerRenderer wrapper.
SandboxLayer::SandboxLayer(Engine::Renderer* displayRenderer) : Engine::Layer("SandboxLayer"), m_DisplayRenderer(displayRenderer)
{
    // Constructor simply stores the renderer pointer.
}

void SandboxLayer::OnAttach()
{
    // Ensure the renderer operates in CPU mode. Future improvement: expose GPU
    // acceleration when available.
    m_Renderer.SetRenderMode(Engine::Renderer::RenderMode::RayTrace);

    // Set up a basic scene with diverse materials.

    // Matte ground to provide a neutral backdrop for other objects.
    std::shared_ptr<Engine::Material> l_GroundMaterial = std::make_shared<Engine::Lambertian>(Vector3{ 0.8f, 0.8f, 0.0f });

    // Diffuse sphere showcasing Lambertian reflection.
    std::shared_ptr<Engine::Material> l_CenterMaterial = std::make_shared<Engine::Lambertian>(Vector3{ 0.1f, 0.2f, 0.5f });

    // Metal sphere to observe reflective behavior with slight roughness.
    std::shared_ptr<Engine::Material> l_MetalMaterial = std::make_shared<Engine::Metal>(Vector3{ 0.8f, 0.8f, 0.8f }, 0.3f);

    // Glass-like sphere demonstrating refraction.
    std::shared_ptr<Engine::Material> l_GlassMaterial = std::make_shared<Engine::Dielectric>(1.5f);

    // Create spheres with the materials defined above.
    Engine::Sphere l_GroundSphere{ Vector3{ 0.0f, -100.5f, -1.0f }, 100.0f, l_GroundMaterial }; // Large sphere acting as a ground plane
    Engine::Sphere l_CenterSphere{ Vector3{ 0.0f, 0.0f, -1.0f }, 0.5f, l_CenterMaterial };       // Central Lambertian sphere
    Engine::Sphere l_LeftSphere{ Vector3{ -1.0f, 0.0f, -1.0f }, 0.5f, l_GlassMaterial };         // Left sphere with glass material
    Engine::Sphere l_RightSphere{ Vector3{ 1.0f, 0.0f, -1.0f }, 0.5f, l_MetalMaterial };         // Right sphere with metal material

    m_Scene.AddSphere(l_GroundSphere);
    m_Scene.AddSphere(l_CenterSphere);
    m_Scene.AddSphere(l_LeftSphere);
    m_Scene.AddSphere(l_RightSphere);
}

void SandboxLayer::OnDetach()
{
    // No special cleanup required for this layer.
}

void SandboxLayer::OnUpdate(float deltaTime)
{
    // deltaTime is currently unused but kept for future timing logic.
    (void)deltaTime;

    // Launch a new render when requested and no render is in progress.
    // The generic Renderer now performs the ray-tracing work directly so
    // we invoke StartRender() on it and pass the current camera from the
    // display renderer.  This keeps the preview and final render in sync.
    if (m_RequestRender && !m_Renderer.IsRendering())
    {
        m_RenderProgress = 0.0f; // Reset progress for upcoming render
        m_RenderTime = 0.0f;     // Clear timing from previous render
        m_Renderer.StartRender(m_Scene, m_DisplayRenderer->GetCamera());
        m_RequestRender = false;
    }

    // Query current progress from the renderer every frame.
    m_RenderProgress = m_Renderer.GetProgress();

    // When rendering is finished, sum each step's time to compute total.
    if (!m_Renderer.IsRendering() && m_RenderProgress >= 1.0f)
    {
        std::vector<Engine::RenderStep> l_Steps = m_Renderer.GetRenderSteps();
        float l_TotalMs = 0.0f; // Accumulated elapsed time for all steps
        for (const Engine::RenderStep& it_Step : l_Steps)
        {
            l_TotalMs += it_Step.m_ElapsedMs;
        }
    
        m_RenderTime = l_TotalMs;
    }
}

void SandboxLayer::OnImGuiRender()
{
    // Create a dock space for ImGui windows; older ImGui versions require an explicit viewport ID.
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID, ImGui::GetMainViewport());

    // Scene viewport displaying the renderer's output texture.
    ImGui::Begin("Scene", nullptr);
    ImVec2 l_Available = ImGui::GetContentRegionAvail();
    const Texture2D& l_Texture = m_DisplayRenderer->GetFrameTexture();
    // rlImGuiImageSize expects a pointer to the texture and integer dimensions,
    // so the available float size is cast accordingly.
    rlImGuiImageSize(&l_Texture, static_cast<int>(l_Available.x), static_cast<int>(l_Available.y)); // TODO: add camera gizmos
    ImGui::End();

    ImGui::Begin("Render Stats", nullptr);

    // Show progress as a bar; width is automatic, height uses default.
    ImGui::ProgressBar(m_RenderProgress, ImVec2(-FLT_MIN, 0.0f));

    // Button to trigger a new render pass.
    if (ImGui::Button("Render"))
    {
        m_RequestRender = true;
    }
    ImGui::End();

    // Display CPU and memory statistics in a dedicated window.
    ImGui::Begin("Resource Usage", nullptr);

    size_t l_MemoryBytes = Engine::Utilities::ResourceStats::GetMemoryUsage();
    float l_MemoryMB = static_cast<float>(l_MemoryBytes) / (1024.0f * 1024.0f);
    float l_CPUUsage = Engine::Utilities::ResourceStats::GetCPUUsage();

    ImGui::Text("Memory: %.2f MB", l_MemoryMB);
    ImGui::Text("CPU: %.2f %%", l_CPUUsage);
    ImGui::End();

    // Present detailed timing for each stage of the render inside its own window.
    // Obtain a copy of the recorded steps; the renderer guards its internal list
    // with a mutex so this snapshot is safe to iterate while threads may still
    // be producing data.
    std::vector<Engine::RenderStep> l_Steps = m_Renderer.GetRenderSteps();

    // Compute tile information once for display outside the table.
    int l_TileSize = m_DisplayRenderer->GetTileSize(); // Future: expose tile sizing to the editor
    int l_TilesX = (GetScreenWidth() + l_TileSize - 1) / l_TileSize;
    int l_TilesY = (GetScreenHeight() + l_TileSize - 1) / l_TileSize;
    int l_TotalTiles = l_TilesX * l_TilesY;
    int l_Completed = static_cast<int>(m_RenderProgress * static_cast<float>(l_TotalTiles));

    // Set a sensible default width and allow window to grow with content
    ImGui::SetNextWindowSize(ImVec2(320.0f, 0.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Render Steps", nullptr);

    // Display overall render time and tile completion once above the table.
    ImGui::Text("Render Time: %.2f ms", m_RenderTime);
    ImGui::Text("Tiles: %d/%d", l_Completed, l_TotalTiles);

    // Two-column table showing each render stage and the CPU time spent.
    if (ImGui::BeginTable("Steps", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        // Column 0: Name of the render stage
        ImGui::TableSetupColumn("Stage", ImGuiTableColumnFlags_WidthStretch);
        // Column 1: Time spent on the CPU for the stage
        ImGui::TableSetupColumn("CPU Time (ms)", ImGuiTableColumnFlags_WidthStretch);
        // Future: GPU/IO timing columns can be added here when available
        ImGui::TableHeadersRow();

        for (const Engine::RenderStep& it_Step : l_Steps)
        {
            // Each row lists the name of the step and its elapsed CPU time.
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", it_Step.m_Name.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", it_Step.m_ElapsedMs);
        }

        // TODO: Display GPU/IO steps once available.
        ImGui::EndTable();
    }
    ImGui::End();
}

void SandboxLayer::OnSceneRender()
{
    if (m_Renderer.IsRendering())
    {
        // Render mode: display progressive framebuffer generated by the ray tracer.
        // Display the progressively rendered framebuffer.
        const Image& l_Image = m_Renderer.GetFrame();
        m_DisplayRenderer->RenderImage(static_cast<const Color*>(l_Image.data), l_Image.width, l_Image.height);
    }

    else
    {
        // Editor mode: show real-time preview using the display renderer's camera.
        m_DisplayRenderer->Begin3D(m_DisplayRenderer->GetCamera());
        // Preview drawing could be added here in the future.
        m_DisplayRenderer->End3D();
    }

    // The render texture is now presented in the Scene viewport; no direct screen blit.
    // TODO: Support pause/resume, saving the final image, and interactive viewport tools.
}