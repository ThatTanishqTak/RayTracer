#pragma once

#include <raylib.h>
#include <memory>

namespace Engine
{
    class Ray;
    class Material;

    /**
     *\brief Stores information about a ray-object intersection.
     */
    struct HitRecord
    {
        Vector3 m_Point{}; /// Point of intersection.
        Vector3 m_Normal{}; /// Surface normal at the hit point.
        float m_Target{}; /// Ray parameter t at the intersection.
        bool m_FrontFace{}; /// rue if the ray hit the front face.
        std::shared_ptr<Material> m_Material{}; /// Material of the hit object.

        /**\brief Determine whether the ray hit front or back face and set the normal accordingly.*/
        void SetFaceNormal(const Ray& ray, const Vector3& outwardNormal);
    };
}