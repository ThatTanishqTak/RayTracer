#pragma once

#include "Tracer/Ray.h"
#include "Tracer/HitRecord.h"

#include <raylib.h>

namespace Engine
{
    /**\brief Abstract base class for all materials.*/
    class Material
    {
    public:
        virtual ~Material() = default;
        /**
         *\brief Compute how an incoming ray scatters when hitting the material.
         *\return True if a scattered ray is produced.
         */
        virtual bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const = 0;
    };

    /**\brief Diffuse material using Lambertian reflection.*/
    class Lambertian : public Material
    {
    public:
        explicit Lambertian(Vector3 albedo);
        virtual bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const override;

    private:
        Vector3 m_Albedo{}; ///< Surface color.
    };

    /**\brief Metal material reflecting rays with optional fuzziness.*/
    class Metal : public Material
    {
    public:
        Metal(Vector3 albedo, float fuzz);
        virtual bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const override;

    private:
        Vector3 m_Albedo{}; ///< Reflection color.
        float m_Fuzz{};     ///< Degree of imperfect reflection.
    };

    /**\brief Transparent material simulating refraction.*/
    class Dielectric : public Material
    {
    public:
        explicit Dielectric(float indexOfRefraction);
        virtual bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const override;

    private:
        float m_IndexOfRefraction{}; ///< Refractive index of the material.

        /**\brief Helper computing reflection probability using Schlick's approximation.*/
        static float Reflectance(float cosine, float refIdx);
    };
}