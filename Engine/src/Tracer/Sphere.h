#pragma once

#include "Tracer/Ray.h"
#include "Tracer/HitRecord.h"

#include <raylib.h>
#include <memory>

namespace Engine
{
    class Material;

    class Sphere
    {
    public:
        Sphere() = default;
        Sphere(Vector3 center, float radius, std::shared_ptr<Material> material);

        const Vector3& GetCenter() const;
        float GetRadius() const;
        std::shared_ptr<Material> GetMaterial() const;

    private:
        Vector3 m_Center{};
        float m_Radius{};
        std::shared_ptr<Material> m_Material{};
    };

    bool RayIntersectsSphere(const Ray& ray, const Sphere& sphere, float targetMinimum, float targetMaximim, HitRecord& hitRecord);
}