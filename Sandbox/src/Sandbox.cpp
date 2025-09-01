#include "SandboxLayer.h"

#include "Core/EntryPoint.h"

class Sandbox : public Engine::Application
{
public:
	Sandbox(const Engine::ApplicationSpecifications& specifications) : Engine::Application(specifications)
	{

	}
};

Engine::Application* Engine::CreateApplication()
{
	ApplicationSpecifications l_Specifications;
	l_Specifications.Title = "RayTracer";

	return new Sandbox(l_Specifications);
}