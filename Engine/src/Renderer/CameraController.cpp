#include "Renderer/CameraController.h"

#include <raymath.h>

namespace Engine
{
    CameraController::CameraController()
    {
        // Default to a simple perspective camera looking at the origin.
        m_Camera.position = { 0.0f, 10.0f, 10.0f };
        m_Camera.target = { 0.0f, 0.0f, 0.0f };
        m_Camera.up = { 0.0f, 1.0f, 0.0f };
        m_Camera.fovy = 45.0f;
        m_Camera.projection = CAMERA_PERSPECTIVE;
    }

    void CameraController::Update(float deltaTime)
    {
        ImGuiIO& l_IO = ImGui::GetIO(); // Query ImGui for current UI state
        bool l_CaptureInput = l_IO.WantCaptureMouse || l_IO.WantCaptureKeyboard;

        // Camera controls are paused when the UI has focus.
        if (l_CaptureInput)
        {
            return;
        }

        // Track cursor state so locking only happens on button state changes.
        // Remember the mouse location so the cursor can return after unlocking.
        if (!l_CaptureInput && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            // Capture the cursor position prior to locking for later restoration.
            Vector2 l_PreviousMousePos = GetMousePosition();
            m_PreviousMousePos = l_PreviousMousePos;
            DisableCursor();

            m_IsCursorLocked = true;
        }

        // On release, unlock the cursor and restore the previous position to avoid snapping.
        if (!l_CaptureInput && IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
        {
            EnableCursor();
            SetMousePosition(static_cast<int>(m_PreviousMousePos.x), static_cast<int>(m_PreviousMousePos.y));

            m_IsCursorLocked = false;
        }

        // Only update the camera when the cursor is locked.
        if (m_IsCursorLocked)
        {
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
                m_Camera.position = Vector3Add(m_Camera.position, Vector3Scale(l_Forward, l_Speed * deltaTime));
            }

            // S: move backward
            if (IsKeyDown(KEY_S))
            {
                m_Camera.position = Vector3Subtract(m_Camera.position, Vector3Scale(l_Forward, l_Speed * deltaTime));
            }

            // A: move left
            if (IsKeyDown(KEY_A))
            {
                m_Camera.position = Vector3Subtract(m_Camera.position, Vector3Scale(l_Right, l_Speed * deltaTime));
            }

            // D: move right
            if (IsKeyDown(KEY_D))
            {
                m_Camera.position = Vector3Add(m_Camera.position, Vector3Scale(l_Right, l_Speed * deltaTime));
            }

            // Q: move down
            if (IsKeyDown(KEY_Q))
            {
                m_Camera.position = Vector3Subtract(m_Camera.position, Vector3Scale(l_Up, l_Speed * deltaTime));
            }

            // E: move up
            if (IsKeyDown(KEY_E))
            {
                m_Camera.position = Vector3Add(m_Camera.position, Vector3Scale(l_Up, l_Speed * deltaTime));
            }

            m_Camera.target = Vector3Add(m_Camera.position, l_Forward); // Update target after movement
        }

        // Future improvement: allow custom key mappings and adjustable rotation speed.
    }

    const Camera& CameraController::GetCamera() const
    {
        return m_Camera;
    }

    Camera& CameraController::GetCamera()
    {
        return m_Camera;
    }
}