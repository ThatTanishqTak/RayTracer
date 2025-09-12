#include "Tracer/BVHNode.h"
#include "Renderer/Material.h"

#include <algorithm>
#include <random>
#include <raymath.h>

namespace Engine
{
    inline float GetAxisComponent(const Vector3& vector, int axis)
    {
        // Access Vector3 components by index; relies on x, y, z being contiguous.
        return (&vector.x)[axis];
    }

    // Helper to compute bounding box of a sphere.
    static BoundingBox SphereBoundingBox(const Sphere& sphere)
    {
        Vector3 l_RadiusVec{ sphere.GetRadius(), sphere.GetRadius(), sphere.GetRadius() };
        BoundingBox l_Box(Vector3Subtract(sphere.GetCenter(), l_RadiusVec), Vector3Add(sphere.GetCenter(), l_RadiusVec));

        return l_Box;
    }

    BVHNode::BVHNode(std::vector<Sphere>& spheres, size_t start, size_t end, std::mt19937& randomEngine)
    {
        // Build the tree by recursively partitioning the list of spheres.
        size_t l_ObjectSpan = end - start;

        if (l_ObjectSpan == 1)
        {
            m_Sphere = spheres[start];
            m_BoundingBox = SphereBoundingBox(m_Sphere);
            m_IsLeaf = true;

            return;
        }

        // Choose axis randomly for splitting using the supplied engine.
        std::uniform_int_distribution<int> l_AxisDistribution(0, 2);
        int l_Axis = l_AxisDistribution(randomEngine); // TODO: allow alternative heuristics for axis selection
        auto a_Comparator = [l_Axis](const Sphere& sphereA, const Sphere& sphereB)
            {
                // Compare sphere centers along the chosen axis.
                return GetAxisComponent(sphereA.GetCenter(), l_Axis) < GetAxisComponent(sphereB.GetCenter(), l_Axis);
            };

        std::sort(spheres.begin() + static_cast<long>(start), spheres.begin() + static_cast<long>(end), a_Comparator);

        size_t l_Mid = start + l_ObjectSpan / 2;
        m_Left = std::make_shared<BVHNode>(spheres, start, l_Mid, randomEngine);
        m_Right = std::make_shared<BVHNode>(spheres, l_Mid, end, randomEngine);

        m_BoundingBox = SurroundingBox(m_Left->m_BoundingBox, m_Right->m_BoundingBox);
    }

    void BVHNode::Rebuild(std::vector<Sphere>& spheres, std::mt19937& randomEngine)
    {
        // Reconstruct the node in place by assigning a freshly built tree.
        *this = BVHNode(spheres, 0, spheres.size(), randomEngine);
    }

    bool BVHNode::Hit(const Ray& ray, float targetMinimum, float targetMaximum, HitRecord& record) const
    {
        if (!m_BoundingBox.Hit(ray, targetMinimum, targetMaximum))
        {
            return false;
        }

        if (m_IsLeaf)
        {
            return RayIntersectsSphere(ray, m_Sphere, targetMinimum, targetMaximum, record);
        }

        HitRecord l_LeftRecord{};
        bool l_HitLeft = m_Left && m_Left->Hit(ray, targetMinimum, targetMaximum, l_LeftRecord);
        float l_NewMax = l_HitLeft ? l_LeftRecord.m_Target : targetMaximum;
        HitRecord l_RightRecord{};
        bool l_HitRight = m_Right && m_Right->Hit(ray, targetMinimum, l_NewMax, l_RightRecord);

        if (l_HitLeft && l_HitRight)
        {
            record = l_RightRecord.m_Target < l_LeftRecord.m_Target ? l_RightRecord : l_LeftRecord;

            return true;
        }

        if (l_HitLeft)
        {
            record = l_LeftRecord;

            return true;
        }

        if (l_HitRight)
        {
            record = l_RightRecord;

            return true;
        }

        return false;
    }

    std::uint32_t BVHNode::WriteNode(std::vector<BVHFlatNode>& outNodes, std::vector<SphereGPU>& outPrimitives, std::vector<MaterialGPU>& outMaterials) const
    {
        // Allocate a slot for this node in the flattened array.
        std::uint32_t l_Index = static_cast<std::uint32_t>(outNodes.size());
        outNodes.push_back(BVHFlatNode{});
        BVHFlatNode& l_Flat = outNodes[l_Index];
        l_Flat.m_Bounds = m_BoundingBox;

        if (m_IsLeaf)
        {
            // Leaf nodes reference a primitive index and have no children.
            l_Flat.m_IsLeaf = 1;
            l_Flat.m_Left = 0;
            l_Flat.m_Right = 0;
            l_Flat.m_Primitive = static_cast<std::uint32_t>(outPrimitives.size());

            // Serialize primitive data for GPU consumption.
            SphereGPU l_GpuSphere{};
            l_GpuSphere.m_Center = m_Sphere.GetCenter();
            l_GpuSphere.m_Radius = m_Sphere.GetRadius();
            l_GpuSphere.m_MaterialIndex = static_cast<std::uint32_t>(outMaterials.size());
            outPrimitives.push_back(l_GpuSphere);

            // Extract material parameters into the parallel material array.
            MaterialGPU l_GpuMaterial{};
            if (std::shared_ptr<Material> l_Material = m_Sphere.GetMaterial())
            {
                l_GpuMaterial.m_Albedo = l_Material->GetAlbedo();
                l_GpuMaterial.m_Fuzz = l_Material->GetFuzz();
                l_GpuMaterial.m_Emission = l_Material->GetEmission();
                l_GpuMaterial.m_RefIdx = l_Material->GetIOR();
                l_GpuMaterial.m_Type = static_cast<std::uint32_t>(l_Material->GetType());
            }
            outMaterials.push_back(l_GpuMaterial);
        }

        else
        {
            // Recursively write children first to obtain their indices.
            std::uint32_t l_LeftIndex = m_Left ? m_Left->WriteNode(outNodes, outPrimitives, outMaterials) : 0;
            std::uint32_t l_RightIndex = m_Right ? m_Right->WriteNode(outNodes, outPrimitives, outMaterials) : 0;
            l_Flat.m_IsLeaf = 0;
            l_Flat.m_Left = l_LeftIndex;
            l_Flat.m_Right = l_RightIndex;
            l_Flat.m_Primitive = 0u;
        }

        return l_Index;
    }

    void BVHNode::FlattenBVH(std::vector<BVHFlatNode>& outNodes, std::vector<SphereGPU>& outPrimitives, std::vector<MaterialGPU>& outMaterials) const
    {
        outNodes.clear();
        outPrimitives.clear();
        outMaterials.clear();

        // Recursively linearize the hierarchy starting from this node.
        WriteNode(outNodes, outPrimitives, outMaterials);
    }
}