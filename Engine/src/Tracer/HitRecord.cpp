#include "Tracer/HitRecord.h"
#include "Tracer/Ray.h"

#include <raymath.h>

namespace Engine
{
    void HitRecord::SetFaceNormal(const Ray& ray, const Vector3& outwardNormal)
    {
        bool l_IsFrontFace = Vector3DotProduct(ray.GetDirection(), outwardNormal) < 0.0f;
        m_FrontFace = l_IsFrontFace;
        m_Normal = l_IsFrontFace ? outwardNormal : Vector3Negate(outwardNormal);
    }
}