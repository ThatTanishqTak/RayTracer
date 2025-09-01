#include "Renderer/Renderer.h"

#include <raylib.h>
#include <rlImGui.h>

namespace Engine
{
	bool Renderer::Initialize()
	{
		bool l_Result = true;
		rlImGuiSetup(l_Result);

		return l_Result;
	}

	void Renderer::Shutdown()
	{
		rlImGuiShutdown();
	}

	void Renderer::BeginFrame()
	{
		BeginDrawing();
		rlImGuiBegin();
		ClearBackground(BLACK);
	}
	
	void Renderer::EndFrame()
	{
		rlImGuiEnd();
		EndDrawing();
	}
}