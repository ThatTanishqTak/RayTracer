#include "Tracer/Ray.h"

#include <raymath.h>

namespace Engine
{
    Ray::Ray(Vector3 origin, Vector3 direction) : m_Origin(origin), m_Direction(direction)
    {
        // Store supplied origin and direction without modification.
    }

    const Vector3& Ray::GetOrigin() const
    {
        return m_Origin;
    }

    const Vector3& Ray::GetDirection() const
    {
        return m_Direction;
    }

    Vector3 Ray::At(float target) const
    {
        // Calculate a point along the ray using parametric form.
        Vector3 l_Result = Vector3Add(m_Origin, Vector3Scale(m_Direction, target));

        return l_Result;
    }
}