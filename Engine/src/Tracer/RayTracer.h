#pragma once

#include "Tracer/Ray.h"
#include "Tracer/BVHNode.h"

#include <raylib.h>

namespace Engine
{
    /**
     *\brief Recursively trace a ray through the world and compute its color contribution.
     */
    Vector3 RayColor(const Ray& ray, const BVHNode& world, int depth);
}