#include "Renderer/Material.h"

#include <raymath.h>

#include <random>
#include <cmath>

namespace
{
    // Generate a random float in the range [0,1).
    float RandomFloat()
    {
        static std::uniform_real_distribution<float> s_Distribution(0.0f, 1.0f);
        static std::mt19937 s_Generator(std::random_device{}());
        float l_Value = s_Distribution(s_Generator);

        return l_Value;
    }

    // Generate a random point inside the unit sphere using rejection sampling.
    Vector3 RandomInUnitSphere()
    {
        while (true)
        {
            Vector3 l_Point{ RandomFloat() * 2.0f - 1.0f, RandomFloat() * 2.0f - 1.0f, RandomFloat() * 2.0f - 1.0f };
            if (Vector3LengthSqr(l_Point) >= 1.0f)
            {
                continue;
            }

            return l_Point;
        }
    }

    // Return a random unit vector by normalizing a random point within the sphere.
    Vector3 RandomUnitVector()
    {
        Vector3 l_Vector = Vector3Normalize(RandomInUnitSphere());

        return l_Vector;
    }
}

namespace Engine
{
    Lambertian::Lambertian(Vector3 albedo) : m_Albedo(albedo)
    {
        // Store the diffuse color of the material.
    }

    bool Lambertian::Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const
    {
        // Scatter rays in a random direction over the hemisphere.
        Vector3 l_ScatterDirection = Vector3Add(hitRecord.m_Normal, RandomUnitVector());
        if (Vector3LengthSqr(l_ScatterDirection) < 1e-8f)
        {
            l_ScatterDirection = hitRecord.m_Normal;
        }

        scattered = Ray(hitRecord.m_Point, l_ScatterDirection);
        attenuation = m_Albedo;

        return true;
    }

    Metal::Metal(Vector3 albedo, float fuzz) : m_Albedo(albedo), m_Fuzz(fuzz < 1.0f ? fuzz : 1.0f)
    {
        // Clamp fuzziness to [0,1] for stability.
    }

    bool Metal::Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const
    {
        // Reflect the incoming ray and perturb it using fuzz.
        Vector3 l_Reflected = Vector3Reflect(Vector3Normalize(rayIn.GetDirection()), hitRecord.m_Normal);
        Vector3 l_ScatterDir = Vector3Add(l_Reflected, Vector3Scale(RandomInUnitSphere(), m_Fuzz));
        scattered = Ray(hitRecord.m_Point, l_ScatterDir);
        attenuation = m_Albedo;
        bool l_Result = Vector3DotProduct(scattered.GetDirection(), hitRecord.m_Normal) > 0.0f;

        return l_Result;
    }

    Dielectric::Dielectric(float indexOfRefraction) : m_IndexOfRefraction(indexOfRefraction)
    {
        // Store optical density for Snell's law calculations.
    }

    bool Dielectric::Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const
    {
        attenuation = { 1.0f, 1.0f, 1.0f };
        float l_RefractionRatio = hitRecord.m_FrontFace ? (1.0f / m_IndexOfRefraction) : m_IndexOfRefraction;
        Vector3 l_UnitDirection = Vector3Normalize(rayIn.GetDirection());
        float l_CosTheta = fminf(Vector3DotProduct(Vector3Negate(l_UnitDirection), hitRecord.m_Normal), 1.0f);
        float l_SinTheta = sqrtf(1.0f - l_CosTheta * l_CosTheta);

        bool l_CannotRefract = l_RefractionRatio * l_SinTheta > 1.0f;
        Vector3 l_Direction{};
        if (l_CannotRefract || Reflectance(l_CosTheta, l_RefractionRatio) > RandomFloat())
        {
            // Reflect if we cannot refract or by probabilistic reflection.
            l_Direction = Vector3Reflect(l_UnitDirection, hitRecord.m_Normal);
        }

        else
        {
            // Otherwise refract through the surface.
            l_Direction = Vector3Refract(l_UnitDirection, hitRecord.m_Normal, l_RefractionRatio);
        }

        scattered = Ray(hitRecord.m_Point, l_Direction);

        return true;
    }

    float Dielectric::Reflectance(float cosine, float refIdx)
    {
        // Schlick's approximation for reflectance.
        float l_R0 = (1.0f - refIdx) / (1.0f + refIdx);
        l_R0 = l_R0 * l_R0;
        float l_Result = l_R0 + (1.0f - l_R0) * powf((1.0f - cosine), 5.0f);

        return l_Result;
    }
}