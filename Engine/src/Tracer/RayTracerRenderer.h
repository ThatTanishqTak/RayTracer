#pragma once

#include "Renderer/Renderer.h"

namespace Engine
{
    /**
     * \brief Compatibility wrapper for the legacy ray-tracing renderer.
     *
     * All rendering functionality now lives in the Renderer base class. This
     * derived type remains for source compatibility and to allow future
     * specialisation without affecting existing client code.
     */
    class RayTracerRenderer : public Renderer
    {

    };
}