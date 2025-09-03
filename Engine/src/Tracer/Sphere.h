#pragma once

#include "Tracer/Ray.h"
#include "Tracer/HitRecord.h"

#include <raylib.h>

namespace Engine
{
    class Sphere
    {
    public:
        Sphere() = default;
        Sphere(Vector3 center, float radius);

        const Vector3& GetCenter() const;
        float GetRadius() const;

    private:
        Vector3 m_Center{};
        float m_Radius{};
    };

    bool RayIntersectsSphere(const Ray& ray, const Sphere& sphere, float targetMinimum, float targetMaximim, HitRecord& hitRecord);
}