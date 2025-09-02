#pragma once

#include <raylib.h>

namespace Engine
{
	class Renderer
	{
	public:
		bool Initialize();
		void Shutdown();

		void BeginFrame();
		void EndFrame();

		void Begin3D(Camera3D camera);
		void End3D();

		Camera3D* GetCamera();

	private:
		Camera3D m_Camera = { 0 };
	};
}