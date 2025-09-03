#include "Tracer/Ray.h"

#include <raymath.h>

namespace Engine
{
    Ray::Ray(Vector3 origin, Vector3 direction) : m_Origin(origin), m_Direction(direction)
    {

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
        Vector3 l_Result = Vector3Add(m_Origin, Vector3Scale(m_Direction, target));

        return l_Result;
    }
}