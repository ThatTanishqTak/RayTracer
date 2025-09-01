#pragma once

namespace Engine
{
	class Renderer
	{
	public:
		bool Initialize();
		void Shutdown();

		void BeginFrame();
		void EndFrame();
	};
}