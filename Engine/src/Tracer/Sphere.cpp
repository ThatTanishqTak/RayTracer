#include "Tracer/Sphere.h"
#include "Renderer/Material.h"

#include <raymath.h>

#include <cmath>

namespace Engine
{
    Sphere::Sphere(Vector3 center, float radius, std::shared_ptr<Material> material) : m_Center(center), m_Radius(radius), m_Material(material)
    {

    }

    const Vector3& Sphere::GetCenter() const
    {
        return m_Center;
    }

    float Sphere::GetRadius() const
    {
        return m_Radius;
    }

    std::shared_ptr<Material> Sphere::GetMaterial() const
    {
        return m_Material;
    }

    bool RayIntersectsSphere(const Ray& ray, const Sphere& sphere, float targetMinimum, float targetMaximim, HitRecord& hitRecord)
    {
        Vector3 l_OC = Vector3Subtract(ray.GetOrigin(), sphere.GetCenter());
        float l_A = Vector3LengthSqr(ray.GetDirection());
        float l_HalfB = Vector3DotProduct(l_OC, ray.GetDirection());
        float l_C = Vector3LengthSqr(l_OC) - sphere.GetRadius() * sphere.GetRadius();

        float l_Discriminant = l_HalfB * l_HalfB - l_A * l_C;
        if (l_Discriminant < 0.0f)
        {
            return false;
        }

        float l_SqrtD = sqrtf(l_Discriminant);

        float l_Root = (-l_HalfB - l_SqrtD) / l_A;
        if (l_Root < targetMinimum || l_Root > targetMaximim)
        {
            l_Root = (-l_HalfB + l_SqrtD) / l_A;
            if (l_Root < targetMinimum || l_Root > targetMaximim)
            {
                return false;
            }
        }

        hitRecord.m_Target = l_Root;
        hitRecord.m_Point = ray.At(l_Root);
        Vector3 l_OutwardNormal = Vector3Scale(Vector3Subtract(hitRecord.m_Point, sphere.GetCenter()), 1.0f / sphere.GetRadius());
        hitRecord.SetFaceNormal(ray, l_OutwardNormal);
        hitRecord.m_Material = sphere.GetMaterial();

        return true;
    }
}