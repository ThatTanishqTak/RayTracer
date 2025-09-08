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

#include <imgui.h>

#include <cfloat>

SandboxLayer::SandboxLayer(Engine::Renderer* displayRenderer) : Engine::Layer("SandboxLayer"), m_DisplayRenderer(displayRenderer)
{
    // Constructor simply stores the renderer pointer.
}

void SandboxLayer::OnAttach() {
    // Currently nothing to initialize when the layer attaches.
}

void SandboxLayer::OnDetach() {
    // No special cleanup required for this layer.
}

void SandboxLayer::OnUpdate(float deltaTime) {
    // deltaTime is currently unused but kept for future timing logic.
    (void)deltaTime;

    // Launch a new render when requested and no render is in progress.
    if (m_RequestRender && !m_Renderer.IsRendering()) {
        m_RenderProgress = 0.0f; // Reset progress for upcoming render
        m_RenderTime = 0.0f;     // Clear timing from previous render
        m_Renderer.StartRender(m_Scene, m_DisplayRenderer->GetCamera());
        m_RequestRender = false;
    }

    // Query current progress from the renderer every frame.
    m_RenderProgress = m_Renderer.GetProgress();

    // When rendering is finished, sum each step's time to compute total.
    if (!m_Renderer.IsRendering() && m_RenderProgress >= 1.0f) {
        std::vector<Engine::RenderStep> l_Steps = m_Renderer.GetRenderSteps();
        float l_TotalMs = 0.0f; // Accumulated elapsed time for all steps
        for (const Engine::RenderStep& it_Step : l_Steps) {
            l_TotalMs += it_Step.m_ElapsedMs;
        }
        m_RenderTime = l_TotalMs;
    }
}

void SandboxLayer::OnImGuiRender() {
    ImGui::Begin("Render Stats", nullptr);

    // Show progress as a bar; width is automatic, height uses default.
    ImGui::ProgressBar(m_RenderProgress, ImVec2(-FLT_MIN, 0.0f));

    // Button to trigger a new render pass.
    if (ImGui::Button("Render")) {
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
    int l_TileSize =
        m_DisplayRenderer
        ->GetTileSize(); // Future: expose tile sizing to the editor
    int l_TilesX = (GetScreenWidth() + l_TileSize - 1) / l_TileSize;
    int l_TilesY = (GetScreenHeight() + l_TileSize - 1) / l_TileSize;
    int l_TotalTiles = l_TilesX * l_TilesY;
    int l_Completed =
        static_cast<int>(m_RenderProgress * static_cast<float>(l_TotalTiles));

    // Set a sensible default width and allow window to grow with content
    ImGui::SetNextWindowSize(ImVec2(320.0f, 0.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Render Steps", nullptr);

    // Display overall render time and tile completion once above the table.
    ImGui::Text("Render Time: %.2f ms", m_RenderTime);
    ImGui::Text("Tiles: %d/%d", l_Completed, l_TotalTiles);

    // Two-column table showing each render stage and the CPU time spent.
    if (ImGui::BeginTable("Steps", 2,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Column 0: Name of the render stage
        ImGui::TableSetupColumn("Stage", ImGuiTableColumnFlags_WidthStretch);
        // Column 1: Time spent on the CPU for the stage
        ImGui::TableSetupColumn("CPU Time (ms)",
            ImGuiTableColumnFlags_WidthStretch);
        // Future: GPU/IO timing columns can be added here when available
        ImGui::TableHeadersRow();

        for (const Engine::RenderStep& it_Step : l_Steps) {
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

void SandboxLayer::OnSceneRender() {
    if (m_Renderer.IsRendering()) {
        // Render mode: display progressive framebuffer generated by the ray tracer.
        const Image& l_Image = m_Renderer.GetFrame();
        m_DisplayRenderer->RenderImage(static_cast<const Color*>(l_Image.data),
            l_Image.width, l_Image.height);
    }

    else {
        // Editor mode: show real-time preview using the renderer's camera.
        m_DisplayRenderer->Begin3D(m_DisplayRenderer->GetCamera());
        // Preview drawing could be added here in the future.
        m_DisplayRenderer->End3D();
    }

    // Blit whichever image the display renderer currently holds to the screen.
    const Texture2D& l_Texture = m_DisplayRenderer->GetFrameTexture();
    Rectangle l_Source{ 0.0f, 0.0f, static_cast<float>(l_Texture.width),
                       static_cast<float>(-l_Texture.height) };
    DrawTextureRec(l_Texture, l_Source, Vector2{ 0.0f, 0.0f }, WHITE);

    // TODO: Support pause/resume and saving final image
}