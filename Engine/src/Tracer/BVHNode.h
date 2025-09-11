#pragma once

#include "Tracer/BoundingBox.h"
#include "Tracer/Sphere.h"
#include "Tracer/GPUPrimitives.h"

#include <memory>
#include <random>
#include <vector>
#include <cstdint>

namespace Engine
{
    /**
     * \brief Compact node representation used by the GPU.
     * Each node stores indices to its children instead of pointers
     * so the structure can be uploaded linearly to the GPU.
     */
    struct BVHFlatNode
    {
        BoundingBox m_Bounds{};       ///< Bounding box enclosing the node.
        std::uint32_t m_Left{ 0 };    ///< Index of the left child or primitive.
        std::uint32_t m_Right{ 0 };   ///< Index of the right child when not a leaf.
        std::uint32_t m_Primitive{ 0 }; ///< Index of the primitive for leaves, UINT32_MAX otherwise.
        std::uint32_t m_IsLeaf{ 0 };  ///< Non-zero when the node is a leaf.
    };

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

        /**
        * \brief Flatten the tree into a linear array of nodes and primitives.
        * The resulting arrays contain child indices, suitable for GPU traversal.
        */
        void FlattenBVH(std::vector<BVHFlatNode>& outNodes, std::vector<SphereGPU>& outPrimitives, std::vector<MaterialGPU>& outMaterials) const;

    private:
        std::shared_ptr<BVHNode> m_Left{};   ///< Left child in the tree.
        std::shared_ptr<BVHNode> m_Right{};  ///< Right child in the tree.
        Sphere m_Sphere{};
        BoundingBox m_BoundingBox{};
        bool m_IsLeaf{ false };

        /**
        * \brief Recursive helper used by FlattenBVH to linearize the hierarchy.
        * @return Index of the newly written flat node.
        */
        std::uint32_t WriteNode(std::vector<BVHFlatNode>& outNode, std::vector<SphereGPU>& outPrimitives, std::vector<MaterialGPU>& outMaterials) const;
    };
}