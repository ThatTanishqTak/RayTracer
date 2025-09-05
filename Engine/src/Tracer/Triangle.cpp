#include "Tracer/Triangle.h"

#include <cmath>

namespace Engine
{
    bool RayIntersectsTriangleSIMD(const Ray& ray, const Triangle& triangle, float& outDistance)
    {
        using namespace DirectX;

        XMVECTOR l_Origin = XMVectorSet(ray.GetOrigin().x, ray.GetOrigin().y, ray.GetOrigin().z, 0.0f);
        XMVECTOR l_Direction = XMVectorSet(ray.GetDirection().x, ray.GetDirection().y, ray.GetDirection().z, 0.0f);

        XMVECTOR l_V0 = XMVectorSet(triangle.m_V0.x, triangle.m_V0.y, triangle.m_V0.z, 0.0f);
        XMVECTOR l_V1 = XMVectorSet(triangle.m_V1.x, triangle.m_V1.y, triangle.m_V1.z, 0.0f);
        XMVECTOR l_V2 = XMVectorSet(triangle.m_V2.x, triangle.m_V2.y, triangle.m_V2.z, 0.0f);

        XMVECTOR l_Edge1 = XMVectorSubtract(l_V1, l_V0);
        XMVECTOR l_Edge2 = XMVectorSubtract(l_V2, l_V0);
        XMVECTOR l_PVec = XMVector3Cross(l_Direction, l_Edge2);
        float l_Determinant = XMVectorGetX(XMVector3Dot(l_Edge1, l_PVec));

        if (fabsf(l_Determinant) < 1e-8f)
        {
            return false; // Ray is parallel to triangle.
        }

        float l_InvDeterminant = 1.0f / l_Determinant;
        XMVECTOR l_TVec = XMVectorSubtract(l_Origin, l_V0);
        float l_U = XMVectorGetX(XMVector3Dot(l_TVec, l_PVec)) * l_InvDeterminant;
        if (l_U < 0.0f || l_U > 1.0f)
        {
            return false;
        }

        XMVECTOR l_QVec = XMVector3Cross(l_TVec, l_Edge1);
        float l_V = XMVectorGetX(XMVector3Dot(l_Direction, l_QVec)) * l_InvDeterminant;
        if (l_V < 0.0f || l_U + l_V > 1.0f)
        {
            return false;
        }

        float l_T = XMVectorGetX(XMVector3Dot(l_Edge2, l_QVec)) * l_InvDeterminant;
        if (l_T < 0.0f)
        {
            return false; // Intersection behind the ray origin.
        }

        outDistance = l_T;

        return true;
    }
}