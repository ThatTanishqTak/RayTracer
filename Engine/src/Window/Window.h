#pragma once

namespace Engine
{
	class Window
	{
	public:
		bool Initialize(int width, int height, const char* title);
		void Shutdown();

		bool ShouldClose() const;
	};
}