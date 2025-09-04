#pragma once

#include "Tracer/Ray.h"
#include "Tracer/Sphere.h"

#include <raylib.h>
#include <vector>

namespace Engine
{
    /**
     *\brief Recursively trace a ray through the world and compute its color contribution.
     */
    Vector3 RayColor(const Ray& ray, const std::vector<Sphere>& world, int depth);
}