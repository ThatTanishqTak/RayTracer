#pragma once

#include "Tracer/Ray.h"
#include "Tracer/HitRecord.h"

#include <raylib.h>

#include <cstdint>

namespace Engine
{
    /**\brief Identifiers for concrete material implementations.
*  Future improvement: move to a dedicated header if materials expand.
*/
    enum class MaterialType : std::uint32_t
    {
        Lambertian = 0, ///< Diffuse surface.
        Metal = 1,      ///< Reflective metal.
        Dielectric = 2  ///< Refractive surface.
    };


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
        
        // Accessors used when flattening materials for GPU consumption.
        virtual Vector3 GetAlbedo() const { return { 0.0f, 0.0f, 0.0f }; }
        virtual float GetFuzz() const { return 0.0f; }
        virtual float GetIOR() const { return 1.0f; }
        virtual MaterialType GetType() const = 0;
    };

    /**\brief Diffuse material using Lambertian reflection.*/
    class Lambertian : public Material
    {
    public:
        explicit Lambertian(Vector3 albedo);
        virtual bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const override;
        
        // GPU flattening accessors
        virtual Vector3 GetAlbedo() const override { return m_Albedo; }
        virtual MaterialType GetType() const override { return MaterialType::Lambertian; }

    private:
        Vector3 m_Albedo{}; ///< Surface color.
    };

    /**\brief Metal material reflecting rays with optional fuzziness.*/
    class Metal : public Material
    {
    public:
        Metal(Vector3 albedo, float fuzz);
        virtual bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const override;
        virtual Vector3 GetAlbedo() const override { return m_Albedo; }
        virtual float GetFuzz() const override { return m_Fuzz; }
        virtual MaterialType GetType() const override { return MaterialType::Metal; }

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
        virtual float GetIOR() const override { return m_IndexOfRefraction; }
        virtual MaterialType GetType() const override { return MaterialType::Dielectric; }

    private:
        float m_IndexOfRefraction{}; ///< Refractive index of the material.

        /**\brief Helper computing reflection probability using Schlick's approximation.*/
        static float Reflectance(float cosine, float refIdx);
    };
}