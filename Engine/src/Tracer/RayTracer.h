#pragma once

#include "Tracer/Ray.h"
#include "Tracer/Sphere.h"

#include <raylib.h>
#include <vector>

namespace Engine
{
    Vector3 RayColor(const Ray& ray, const std::vector<Sphere>& world, int depth);
}