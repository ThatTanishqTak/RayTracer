#include "Scene/Scene.h"

namespace Engine
{
    void Scene::AddSphere(const Sphere& sphere)
    {
        m_Spheres.push_back(sphere);
        RebuildBVH(); // Rebuild acceleration structure after scene change.
    }

    const BVHNode& Scene::GetBVH() const
    {
        return m_BVHRoot;
    }

    const std::vector<Sphere>& Scene::GetSpheres() const
    {
        return m_Spheres;
    }

    void Scene::RebuildBVH()
    {
        if (!m_Spheres.empty())
        {
            m_BVHRoot.Rebuild(m_Spheres, m_RandomEngine);
        }
    }
}