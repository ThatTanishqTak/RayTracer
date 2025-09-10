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

        /**
         * @brief Access the flattened BVH nodes for GPU upload.
         */
        const std::vector<BVHFlatNode>& GetFlatNodes() const;

        /**
         * @brief Access the flattened primitive list for GPU upload.
         */
        const std::vector<Sphere>& GetFlatPrimitives() const;

    private:
        /**
         * @brief Rebuild the BVH after scene modifications.
         */
        void RebuildBVH();

        std::vector<Sphere> m_Spheres{}; ///< Stored scene spheres.
        BVHNode m_BVHRoot{};             ///< Root node of the BVH.
        std::mt19937 m_RandomEngine{ std::random_device{}() }; ///< Random engine for BVH builds.
        std::vector<BVHFlatNode> m_FlatNodes{}; ///< Flattened BVH for GPU traversal.
        std::vector<Sphere> m_FlatPrimitives{}; ///< Flattened primitive array.

        // TODO: Implement instancing & geometry caching
    };
}