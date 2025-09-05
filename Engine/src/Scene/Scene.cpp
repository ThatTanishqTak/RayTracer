#include "Scene/Scene.h"

#include <utility>

namespace Engine
{
    void Scene::AddObject(std::shared_ptr<Object> object)
    {
        m_Objects.push_back(std::move(object));
        // TODO: Implement BVH or other spatial partitioning when scene mutates
    }

    const std::vector<std::shared_ptr<Object>>& Scene::GetObjects() const
    {
        return m_Objects;
    }
}