#include "Tracer/RayTracer.h"
#include "Renderer/Material.h"

#include <raymath.h>

#include <limits>

namespace Engine
{
    Vector3 RayColor(const Ray& ray, const std::vector<Sphere>& world, int depth)
    {
        // Limit recursion to avoid infinite bounces.
        if (depth <= 0)
        {
            Vector3 l_Black{ 0.0f, 0.0f, 0.0f };

            return l_Black;
        }

        HitRecord l_Record{};
        HitRecord l_TempRecord{};
        bool l_HitAnything = false;
        float l_ClosestSoFar = std::numeric_limits<float>::max();

        // Check the ray against every object in the world and keep the closest hit.
        for (const Sphere& it_Sphere : world)
        {
            if (RayIntersectsSphere(ray, it_Sphere, 0.001f, l_ClosestSoFar, l_TempRecord))
            {
                l_HitAnything = true;
                l_ClosestSoFar = l_TempRecord.m_Target;
                l_Record = l_TempRecord;
            }
        }

        if (l_HitAnything)
        {
            Ray l_Scattered;
            Vector3 l_Attenuation{};
            // Delegate scattering to the material and recurse with the scattered ray.
            if (l_Record.m_Material && l_Record.m_Material->Scatter(ray, l_Record, l_Attenuation, l_Scattered))
            {
                Vector3 l_Color = RayColor(l_Scattered, world, depth - 1);
                Vector3 l_Result = Vector3Multiply(l_Color, l_Attenuation);

                return l_Result;
            }

            Vector3 l_Zero{ 0.0f, 0.0f, 0.0f };

            return l_Zero;
        }

        // Render a simple gradient background when no objects are hit.
        Vector3 l_UnitDirection = Vector3Normalize(ray.GetDirection());
        float l_T = 0.5f * (l_UnitDirection.y + 1.0f);
        Vector3 l_Start{ 1.0f, 1.0f, 1.0f };
        Vector3 l_End{ 0.5f, 0.7f, 1.0f };
        Vector3 l_Result = Vector3Add(Vector3Scale(l_Start, 1.0f - l_T), Vector3Scale(l_End, l_T));

        return l_Result;
    }
}