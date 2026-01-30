using System;
using Fermion;

namespace Sandbox
{
    public class CameraRotationTest : Entity
    {
        private TransformComponent m_Transform;
        private float m_Time = 0.0f;

        public void OnCreate()
        {
            Utils.Log("=== CameraRotationTest OnCreate ===");

            if (!HasComponent<TransformComponent>())
            {
                Utils.Log("ERROR: No TransformComponent!");
                return;
            }

            m_Transform = GetComponent<TransformComponent>();

            Vector3 initialRotation = m_Transform.Rotation;
            Utils.Log("Initial Rotation: X=" + initialRotation.X + " Y=" + initialRotation.Y + " Z=" + initialRotation.Z);

            Vector3 initialPosition = m_Transform.Translation;
            Utils.Log("Initial Position: X=" + initialPosition.X + " Y=" + initialPosition.Y + " Z=" + initialPosition.Z);
        }

        public void OnUpdate(float ts)
        {
            if (m_Transform == null)
                return;

            m_Time += ts;

            float yaw = m_Time * 0.5f;  
            float pitch = (float)Math.Sin(m_Time) * 0.3f; 

            m_Transform.Rotation = new Vector3(pitch, yaw, 0.0f);

        
            if ((int)m_Time != (int)(m_Time - ts))
            {
                Vector3 currentRotation = m_Transform.Rotation;
                Utils.Log("Time=" + (int)m_Time + " Rotation: X=" + currentRotation.X + " Y=" + currentRotation.Y);
            }
        }
    }
}
