#pragma once

#include <memory>
#include <vector>

namespace Engine
{
    class Object;

    /**
     * @brief Manages a collection of objects that make up the scene.
     *
     * Use this class to store and access the objects that are part of the
     * current rendering scene.
     */
    class Scene
    {
    public:
        /**
         * @brief Add an object to the scene.
         *
         * Call this method when you want the scene to manage a new object.
         * @param object Shared pointer to the object to add.
         */
        void AddObject(std::shared_ptr<Object> object);

        /**
         * @brief Retrieve the list of objects in the scene.
         *
         * Use this method to iterate over the objects currently stored in the scene.
         * @return Const reference to the vector of object pointers.
         */
        const std::vector<std::shared_ptr<Object>>& GetObjects() const;

    private:
        std::vector<std::shared_ptr<Object>> m_Objects{}; /// Stored scene objects.
    };
}