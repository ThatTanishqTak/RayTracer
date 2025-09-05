#include "Tracer/BVHNode.h"

#include <algorithm>
#include <random>
#include <raymath.h>

namespace Engine
{
    inline float GetAxisComponent(const Vector3& a_Vector, int it_Axis)
    {
        // Access Vector3 components by index; relies on x, y, z being contiguous.
        return (&a_Vector.x)[it_Axis];
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
        auto a_Comparator = [l_Axis](const Sphere& a_SphereA, const Sphere& a_SphereB)
            {
                // Compare sphere centers along the chosen axis.
                return GetAxisComponent(a_SphereA.GetCenter(), l_Axis) < GetAxisComponent(a_SphereB.GetCenter(), l_Axis);
            };

        std::sort(spheres.begin() + static_cast<long>(start), spheres.begin() + static_cast<long>(end), a_Comparator);

        size_t l_Mid = start + l_ObjectSpan / 2;
        m_Left = std::make_shared<BVHNode>(spheres, start, l_Mid, randomEngine);
        m_Right = std::make_shared<BVHNode>(spheres, l_Mid, end, randomEngine);

        m_BoundingBox = SurroundingBox(m_Left->m_BoundingBox, m_Right->m_BoundingBox);
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
}