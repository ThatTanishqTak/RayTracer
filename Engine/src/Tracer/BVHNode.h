#pragma once

#include "Tracer/BoundingBox.h"
#include "Tracer/Sphere.h"

#include <memory>
#include <vector>

namespace Engine
{
    /**
    *\brief Node of a bounding volume hierarchy built from spheres.
    */
    class BVHNode
    {
    public:
        BVHNode() = default;
        BVHNode(std::vector<Sphere>& spheres, size_t start, size_t end);

        /**\brief Test the BVH tree for ray intersections.*/
        bool Hit(const Ray& ray, float targetMinimum, float targetMaximum, HitRecord& record) const;

    private:
        std::shared_ptr<BVHNode> m_Left{};
        std::shared_ptr<BVHNode> m_Right{};
        Sphere m_Sphere{};
        BoundingBox m_BoundingBox{};
        bool m_IsLeaf{ false };
    };
}