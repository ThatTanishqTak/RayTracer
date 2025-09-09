#include "SandboxLayer.h"

#include "Core/EntryPoint.h"
#include "Utilities/Logging.h"

class Sandbox : public Engine::Application
{
public:
    Sandbox(const Engine::ApplicationSpecifications& specifications) : Engine::Application(specifications)
    {
        // Push the main sandbox layer onto the application stack.
        PushLayer(new SandboxLayer(GetRenderer()));
    }
};

Engine::Application* Engine::CreateApplication()
{
    RAY_INFO("-------CREATING APPLICATION-------");

    ApplicationSpecifications l_Specifications;
    l_Specifications.Width = 1920;
    l_Specifications.Height = 1080;
    l_Specifications.Title = "Sandbox";

    RAY_TRACE("Width: {}", l_Specifications.Width);
    RAY_TRACE("Height: {}", l_Specifications.Height);
    RAY_TRACE("Title: {}", l_Specifications.Title);

    RAY_INFO("-------APPLICATION CREATED-------");

    return new Sandbox(l_Specifications);
}