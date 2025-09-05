#pragma once

#include "Tracer/BoundingBox.h"
#include "Tracer/Sphere.h"

#include <memory>
#include <random>
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
        /**\brief Construct the BVH tree using the provided random engine.
        * The engine is passed by reference to allow deterministic but thread-safe builds.
        */
        BVHNode(std::vector<Sphere>& spheres, size_t start, size_t end, std::mt19937& randomEngine);

        /**\brief Rebuild the BVH tree from the supplied sphere list.
        *  Call this whenever the scene geometry mutates.
        */
        void Rebuild(std::vector<Sphere>& spheres, std::mt19937& randomEngine);

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