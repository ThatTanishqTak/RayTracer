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

    const std::vector<BVHFlatNode>& Scene::GetFlatNodes() const
    {
        return m_FlatNodes;
    }

    const std::vector<Sphere>& Scene::GetFlatPrimitives() const
    {
        return m_FlatPrimitives;
    }

    void Scene::RebuildBVH()
    {
        if (!m_Spheres.empty())
        {
            m_BVHRoot.Rebuild(m_Spheres, m_RandomEngine);
            // Convert the pointer-based BVH into a linear array for GPU use.
            m_BVHRoot.FlattenBVH(m_FlatNodes, m_FlatPrimitives);
        }
    }
}