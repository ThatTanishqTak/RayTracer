#include "Renderer/Renderer.h"

#include <raylib.h>

namespace Engine
{
	bool Renderer::Initialize()
	{
		return true;
	}

	void Renderer::Shutdown()
	{

	}

	void Renderer::BeginFrame()
	{
		BeginDrawing();
		ClearBackground(BLACK);
	}
	
	void Renderer::EndFrame()
	{
		EndDrawing();
	}
}