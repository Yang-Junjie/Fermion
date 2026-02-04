using System;
using Fermion;

namespace Photon
{
    public class FPSCameraController : Entity
    {
        public float MouseSensitivity = 0.002f;

        public float PitchMin = -1.5f;
        public float PitchMax = 1.5f; 

        public bool Enabled = true;

        public bool LockCursorOnStart = true;

        public KeyCode ToggleCursorKey = KeyCode.Escape;

        private TransformComponent m_Transform;
        private Vector2 m_LastMousePosition;
        private bool m_CursorLocked = false;
        private bool m_FirstUpdate = true;
        private bool m_LastToggleKeyDown = false;

        private float m_Pitch = 0.0f;
        private float m_Yaw = 0.0f;

        public void OnCreate()
        {
            if (!HasComponent<TransformComponent>())
            {
                Utils.Log("FPSCameraController requires TransformComponent!");
                return;
            }

            m_Transform = GetComponent<TransformComponent>();

            Vector3 currentRotation = m_Transform.Rotation;
            m_Pitch = currentRotation.X;
            m_Yaw = currentRotation.Y;

            if (LockCursorOnStart)
            {
                LockCursor();
            }
        }

        public void OnUpdate(float ts)
        {
            if (m_Transform == null)
                return;

            bool toggleKeyDown = Input.IsKeyDown(ToggleCursorKey);
            if (toggleKeyDown && !m_LastToggleKeyDown)
            {
                if (m_CursorLocked)
                    UnlockCursor();
                else
                    LockCursor();
            }
            m_LastToggleKeyDown = toggleKeyDown;

            if (!Enabled || !m_CursorLocked)
                return;

            Vector2 currentMousePosition = Input.GetMousePosition();

            if (m_FirstUpdate)
            {
                m_LastMousePosition = currentMousePosition;
                m_FirstUpdate = false;
                return;
            }

            float deltaX = currentMousePosition.X - m_LastMousePosition.X;
            float deltaY = currentMousePosition.Y - m_LastMousePosition.Y;

            m_Yaw -= deltaX * MouseSensitivity;

            m_Pitch -= deltaY * MouseSensitivity;  
            m_Pitch = Clamp(m_Pitch, PitchMin, PitchMax);

            m_Transform.Rotation = new Vector3(m_Pitch, m_Yaw, 0.0f);

            m_LastMousePosition = currentMousePosition;
        }

        private void LockCursor()
        {
            Input.SetCursorMode(CursorMode.Locked);
            m_CursorLocked = true;
            m_FirstUpdate = true;  
        }

        private void UnlockCursor()
        {
            Input.SetCursorMode(CursorMode.Normal);
            m_CursorLocked = false;
        }

        private static float Clamp(float value, float min, float max)
        {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }
    }
}
