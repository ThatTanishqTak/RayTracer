#pragma once

#include "Tracer/Ray.h"
#include "Tracer/HitRecord.h"
#include "Tracer/BoundingBox.h"

#include <raylib.h>
#include <memory>

namespace Engine
{
    class Material;

    /**\brief Simple sphere primitive used for ray-scene intersections.*/
    class Sphere
    {
    public:
        Sphere() = default;
        Sphere(Vector3 center, float radius, std::shared_ptr<Material> material);

        /**\brief Access the sphere's center position.*/
        const Vector3& GetCenter() const;
        /**\brief Access the sphere's radius.*/
        float GetRadius() const;
        /**\brief Retrieve the material associated with the sphere.*/
        std::shared_ptr<Material> GetMaterial() const;

        /**\brief Compute an axis aligned bounding box for the sphere.*/
        BoundingBox GetBoundingBox() const;

    private:
        Vector3 m_Center{};
        float m_Radius{};
        std::shared_ptr<Material> m_Material{};
    };

    /**\brief Test whether a ray intersects a sphere within the given range.*/
    bool RayIntersectsSphere(const Ray& ray, const Sphere& sphere, float targetMinimum, float targetMaximim, HitRecord& hitRecord);
}