#include "SandboxLayer.h"

#include "Core/EntryPoint.h"

class Sandbox : public Engine::Application
{

};

Engine::Application* Engine::CreateApplication()
{
	return new Sandbox();
}