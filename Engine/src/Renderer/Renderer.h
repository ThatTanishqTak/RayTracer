#include <iostream>

namespace Engine
{
	class Renderer
	{
	public:
		bool Initialize();
		void Shutdown();

		void DrawFrame();
	};
}