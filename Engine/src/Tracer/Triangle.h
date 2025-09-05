#pragma once

#include "Tracer/Ray.h"

#include <DirectXMath.h>

namespace Engine
{
    /**
     * @brief Simple triangle primitive defined by three vertices.
     */
    struct Triangle
    {
        Vector3 m_V0{};
        Vector3 m_V1{};
        Vector3 m_V2{};
    };

    /**
     * @brief SIMD optimized ray-triangle intersection using DirectXMath.
     * @param ray Incoming ray.
     * @param triangle Triangle to test against.
     * @param outDistance Distance to intersection point if hit.
     * @return True when the ray intersects the triangle.
     */
    bool RayIntersectsTriangleSIMD(const Ray& ray, const Triangle& triangle, float& outDistance);
}