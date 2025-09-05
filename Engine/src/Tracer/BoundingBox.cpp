#include "Tracer/BoundingBox.h"

#include <algorithm>

namespace Engine
{
    inline float GetAxisComponent(const Vector3& vector, int axis)
    {
        // Access Vector3 components by index; relies on x, y, z being contiguous.
        return (&vector.x)[axis];
    }

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
        for (int axis = 0; axis < 3; ++axis)
        {
            float l_InverseDirection = 1.0f / GetAxisComponent(ray.GetDirection(), axis);
            float l_T0 = (GetAxisComponent(m_Min, axis) - GetAxisComponent(ray.GetOrigin(), axis)) * l_InverseDirection;
            float l_T1 = (GetAxisComponent(m_Max, axis) - GetAxisComponent(ray.GetOrigin(), axis)) * l_InverseDirection;

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