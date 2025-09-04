#pragma once

#include "Tracer/Ray.h"

#include <raylib.h>

namespace Engine
{
    /**
    *\brief Axis aligned bounding box used for spatial queries.
    */
    class BoundingBox
    {
    public:
        BoundingBox() = default;
        BoundingBox(Vector3 minPoint, Vector3 maxPoint);

        /**\brief Test if a ray intersects the bounding box within the given range.*/
        bool Hit(const Ray& ray, float targetMinimum, float targetMaximum) const;

        /**\brief Accessors for the minimum corner.*/
        const Vector3& GetMin() const;
        /**\brief Accessors for the maximum corner.*/
        const Vector3& GetMax() const;

    private:
        Vector3 m_Min{};
        Vector3 m_Max{};
    };

    /**\brief Create a bounding box that encompasses two other boxes.*/
    BoundingBox SurroundingBox(const BoundingBox& boxA, const BoundingBox& boxB);
}