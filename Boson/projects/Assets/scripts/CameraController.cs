using System;
using Fermion;

namespace Sandbox
{
    public class CameraController : Entity
    {
        public float MoveSpeed = 50.0f;

        private TransformComponent m_Transform;

        private bool enable = false;
        private bool lastQDown = false;

        public void OnCreate()
        {
            if (!HasComponent<TransformComponent>())
            {
                Utils.Log("CameraController requires TransformComponent!");
                return;
            }

            m_Transform = GetComponent<TransformComponent>();
        }

        public void OnUpdate(float ts)
        {

            if (m_Transform == null)
                return;
            Vector3 translation = m_Transform.Translation;

            float speed = MoveSpeed * ts;

            bool currentQDown = Input.IsKeyDown(KeyCode.Q);
            if (currentQDown && !lastQDown)
            {
                enable = !enable;
            }
            lastQDown = currentQDown;

            if (enable)
            {
                if (Input.IsKeyDown(KeyCode.W) || Input.IsKeyDown(KeyCode.Up))
                    translation.Y += speed;

                if (Input.IsKeyDown(KeyCode.S) || Input.IsKeyDown(KeyCode.Down))
                    translation.Y -= speed;

                if (Input.IsKeyDown(KeyCode.A) || Input.IsKeyDown(KeyCode.Left))
                    translation.X -= speed;

                if (Input.IsKeyDown(KeyCode.D) || Input.IsKeyDown(KeyCode.Right))
                    translation.X += speed;
            }

            m_Transform.Translation = translation;
        }
    }
}
