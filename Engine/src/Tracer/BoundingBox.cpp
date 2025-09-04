#include "Tracer/BoundingBox.h"

#include <algorithm>

namespace Engine
{
    BoundingBox::BoundingBox(Vector3 minPoint, Vector3 maxPoint) : m_Min(minPoint), m_Max(maxPoint)
    {
        // Store bounds for later intersection checks.
    }

    const Vector3& BoundingBox::GetMin() const
    {
        return m_Min;
    }

    const Vector3& BoundingBox::GetMax() const
    {
        return m_Max;
    }

    bool BoundingBox::Hit(const Ray& ray, float targetMinimum, float targetMaximum) const
    {
        // Use the slab method to test intersection with the box on each axis.
        for (int it_Axis = 0; it_Axis < 3; ++it_Axis)
        {
            float l_InverseDirection = 1.0f / ray.GetDirection().v[it_Axis];
            float l_T0 = (m_Min.v[it_Axis] - ray.GetOrigin().v[it_Axis]) * l_InverseDirection;
            float l_T1 = (m_Max.v[it_Axis] - ray.GetOrigin().v[it_Axis]) * l_InverseDirection;

            if (l_InverseDirection < 0.0f)
            {
                std::swap(l_T0, l_T1);
            }

            targetMinimum = l_T0 > targetMinimum ? l_T0 : targetMinimum;
            targetMaximum = l_T1 < targetMaximum ? l_T1 : targetMaximum;
            if (targetMaximum <= targetMinimum)
            {
                return false;
            }
        }

        return true;
    }

    BoundingBox SurroundingBox(const BoundingBox& boxA, const BoundingBox& boxB)
    {
        Vector3 l_Small{ fminf(boxA.GetMin().x, boxB.GetMin().x), fminf(boxA.GetMin().y, boxB.GetMin().y), fminf(boxA.GetMin().z, boxB.GetMin().z) };
        Vector3 l_Big{ fmaxf(boxA.GetMax().x, boxB.GetMax().x), fmaxf(boxA.GetMax().y, boxB.GetMax().y), fmaxf(boxA.GetMax().z, boxB.GetMax().z) };

        BoundingBox l_Box(l_Small, l_Big);

        return l_Box;
    }
}