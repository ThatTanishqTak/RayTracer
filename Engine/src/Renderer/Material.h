#pragma once

#include "Tracer/Ray.h"
#include "Tracer/HitRecord.h"

#include <raylib.h>

namespace Engine
{
    class Material
    {
    public:
        virtual ~Material() = default;
        virtual bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const = 0;
    };

    class Lambertian : public Material
    {
    public:
        explicit Lambertian(Vector3 albedo);
        virtual bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const override;

    private:
        Vector3 m_Albedo{};
    };

    class Metal : public Material
    {
    public:
        Metal(Vector3 albedo, float fuzz);
        virtual bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const override;

    private:
        Vector3 m_Albedo{};
        float m_Fuzz{};
    };

    class Dielectric : public Material
    {
    public:
        explicit Dielectric(float indexOfRefraction);
        virtual bool Scatter(const Ray& rayIn, const HitRecord& hitRecord, Vector3& attenuation, Ray& scattered) const override;

    private:
        float m_IndexOfRefraction{};

        static float Reflectance(float cosine, float refIdx);
    };
}