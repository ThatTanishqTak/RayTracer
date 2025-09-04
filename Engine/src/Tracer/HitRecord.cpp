#include "Tracer/HitRecord.h"
#include "Tracer/Ray.h"

#include <raymath.h>

namespace Engine
{
    void HitRecord::SetFaceNormal(const Ray& ray, const Vector3& outwardNormal)
    {
        // Determine if the hit occurred on the outside or inside of the surface.
        bool l_IsFrontFace = Vector3DotProduct(ray.GetDirection(), outwardNormal) < 0.0f;
        m_FrontFace = l_IsFrontFace;
        // Ensure the normal always points against the ray's direction.
        m_Normal = l_IsFrontFace ? outwardNormal : Vector3Negate(outwardNormal);
    }
}