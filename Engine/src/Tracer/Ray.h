#pragma once

#include <raylib.h>

namespace Engine
{
    /**\brief Represents a mathematical ray with origin and direction.*/
    class Ray
    {
    public:
        Ray() = default;
        /**\brief Construct a ray from an origin point and direction vector.*/
        Ray(Vector3 origin, Vector3 direction);

        /**\brief Access the ray's origin.*/
        const Vector3& GetOrigin() const;
        /**\brief Access the ray's direction.*/
        const Vector3& GetDirection() const;
        /**\brief Compute a point along the ray at distance t.*/
        Vector3 At(float t) const;

    private:
        Vector3 m_Origin{};
        Vector3 m_Direction{};
    };
}