#pragma once

#include <raylib.h>
#include <memory>

namespace Engine
{
    class Ray;
    class Material;

    struct HitRecord
    {
        Vector3 m_Point{};
        Vector3 m_Normal{};
        float m_Target{};
        bool m_FrontFace{};
        std::shared_ptr<Material> m_Material{};

        void SetFaceNormal(const Ray& ray, const Vector3& outwardNormal);
    };
}