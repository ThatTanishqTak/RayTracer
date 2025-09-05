#pragma once

#include "Tracer/Sphere.h"
#include "Tracer/BVHNode.h"

#include <random>
#include <vector>

namespace Engine
{
    /**
     * @brief Manages scene primitives and maintains an acceleration structure.
     */
    class Scene
    {
    public:
        /**
         * @brief Add a sphere to the scene and rebuild the BVH.
         * @param sphere Sphere to insert.
         */
        void AddSphere(const Sphere& sphere);

        /**
         * @brief Access the current BVH root node.
         * @return Const reference to the root BVH node.
         */
        const BVHNode& GetBVH() const;

        /**
         * @brief Retrieve all spheres in the scene.
         * @return Const reference to internal sphere container.
         */
        const std::vector<Sphere>& GetSpheres() const;

    private:
        /**
         * @brief Rebuild the BVH after scene modifications.
         */
        void RebuildBVH();

        std::vector<Sphere> m_Spheres{}; ///< Stored scene spheres.
        BVHNode m_BVHRoot{};             ///< Root node of the BVH.
        std::mt19937 m_RandomEngine{ std::random_device{}() }; ///< Random engine for BVH builds.

        // TODO: Implement instancing & geometry caching
    };
}