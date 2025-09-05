#pragma once

#include <raylib.h>

namespace Engine
{
    /**
     * \brief Handles basic WASDQE camera movement and mouse look.
     *
     * Designed to be extended with editor-specific controls in the future.
     */
    class CameraController
    {
    public:
        /** \brief Update the internal camera based on user input. */
        void Update(float l_DeltaSeconds);

        /** \brief Retrieve the current camera. */
        const Camera& GetCamera() const;

    private:
        Camera m_Camera{};            ///< Internal camera state used for rendering.
        float m_MoveSpeed{ 5.0f };      ///< Units per second movement speed. // TODO: expose move speed to editor UI
    };
}