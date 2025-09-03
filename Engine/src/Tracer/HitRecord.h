#pragma once

#include <raylib.h>

namespace Engine
{
    class Ray;

    struct HitRecord
    {
        Vector3 m_Point{};
        Vector3 m_Normal{};
        float m_Target{};
        bool m_FrontFace{};

        void SetFaceNormal(const Ray& ray, const Vector3& outwardNormal);
    };
}