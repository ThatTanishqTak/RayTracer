#pragma once

#include <raylib.h>
#include <imgui.h>

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
        /** \brief Construct controller with a default perspective camera. */
        CameraController();
        /** \brief Update the internal camera based on user input.
         *
         * Camera controls are skipped when the UI captures keyboard or mouse
         * input.
         */
        void Update(float deltaTime);

        /** \brief Retrieve the current camera for read-only access. */
        const Camera& GetCamera() const;
        /** \brief Retrieve the current camera for modification. */
        Camera& GetCamera();

    private:
        Camera m_Camera{};            ///< Internal camera state used for rendering.
        float m_MoveSpeed{ 5.0f };      ///< Units per second movement speed. // TODO: expose move speed to editor UI
        bool m_IsCursorLocked{ false }; ///< Tracks whether the cursor is currently locked for camera look.
        Vector2 m_PreviousMousePos{};   ///< Stored cursor position prior to locking. // TODO: allow disabling cursor return
    };
}