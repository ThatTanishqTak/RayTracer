#pragma once

#include <raylib.h>

namespace Engine
{
    class Ray
    {
    public:
        Ray() = default;
        Ray(Vector3 origin, Vector3 direction);

        const Vector3& GetOrigin() const;
        const Vector3& GetDirection() const;
        Vector3 At(float t) const;

    private:
        Vector3 m_Origin{};
        Vector3 m_Direction{};
    };
}