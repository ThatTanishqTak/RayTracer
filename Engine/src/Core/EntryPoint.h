#pragma once

#include "Core/Application.h"

namespace Engine
{
    /**\brief Factory function implemented by the client to create its application.*/
    Application* CreateApplication();
}

/**
 *\brief Program entry point. Delegates creation and execution to the client application.
 */
int main()
{
    auto a_App = Engine::CreateApplication();
    a_App->Run();

    delete a_App;

    return 0;
}