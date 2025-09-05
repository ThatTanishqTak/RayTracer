#include "Scene/Scene.h"

#include <utility>

namespace Engine
{
    void Scene::AddObject(std::shared_ptr<Object> l_Object)
    {
        m_Objects.push_back(std::move(l_Object));
        // TODO: Implement BVH or other spatial partitioning when scene mutates
    }

    const std::vector<std::shared_ptr<Object>>& Scene::GetObjects() const
    {
        return m_Objects;
    }
}