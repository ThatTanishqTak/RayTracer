#include "Renderer/CameraController.h"

#include <raymath.h>

namespace Engine
{
    void CameraController::Update(float l_DeltaSeconds)
    {
        // Right mouse button enables camera controls.
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            DisableCursor();

            // Mouse look: adjust forward direction from mouse movement.
            Vector2 l_MouseDelta = GetMouseDelta();
            float l_RotateSpeed = 0.1f;
            Vector3 l_Forward = Vector3Subtract(m_Camera.target, m_Camera.position);
            Matrix l_Rotation = MatrixRotateXYZ(Vector3{ -l_MouseDelta.y * DEG2RAD * l_RotateSpeed, -l_MouseDelta.x * DEG2RAD * l_RotateSpeed, 0.0f });
            l_Forward = Vector3Transform(l_Forward, l_Rotation);

            Vector3 l_Right = Vector3Normalize(Vector3CrossProduct(l_Forward, m_Camera.up));
            Vector3 l_Up = Vector3Normalize(Vector3CrossProduct(l_Right, l_Forward));

            float l_Speed = m_MoveSpeed;
            if (IsKeyDown(KEY_LEFT_SHIFT))
            {
                l_Speed *= 2.0f; // Increase speed when shift is held
            }

            // W: move forward
            if (IsKeyDown(KEY_W))
            {
                m_Camera.position = Vector3Add(m_Camera.position, Vector3Scale(l_Forward, l_Speed * l_DeltaSeconds));
            }

            // S: move backward
            if (IsKeyDown(KEY_S))
            {
                m_Camera.position = Vector3Subtract(m_Camera.position, Vector3Scale(l_Forward, l_Speed * l_DeltaSeconds));
            }

            // A: move left
            if (IsKeyDown(KEY_A))
            {
                m_Camera.position = Vector3Subtract(m_Camera.position, Vector3Scale(l_Right, l_Speed * l_DeltaSeconds));
            }

            // D: move right
            if (IsKeyDown(KEY_D))
            {
                m_Camera.position = Vector3Add(m_Camera.position, Vector3Scale(l_Right, l_Speed * l_DeltaSeconds));
            }

            // Q: move down
            if (IsKeyDown(KEY_Q))
            {
                m_Camera.position = Vector3Subtract(m_Camera.position, Vector3Scale(l_Up, l_Speed * l_DeltaSeconds));
            }

            // E: move up
            if (IsKeyDown(KEY_E))
            {
                m_Camera.position = Vector3Add(m_Camera.position, Vector3Scale(l_Up, l_Speed * l_DeltaSeconds));
            }

            m_Camera.target = Vector3Add(m_Camera.position, l_Forward); // Update target after movement
        }

        else
        {
            EnableCursor();
        }

        // Future improvement: allow custom key mappings and adjustable rotation speed.
    }

    const Camera& CameraController::GetCamera() const
    {
        return m_Camera;
    }
}