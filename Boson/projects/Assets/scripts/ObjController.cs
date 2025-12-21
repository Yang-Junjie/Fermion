using System;
using Fermion;

namespace Sandbox
{
    public class ObjController : Entity
    {
        public float MoveSpeed = 10.0f;

        private TransformComponent m_Transform;

        private bool enable = false;
        private bool lastQDown = false;

        public void OnCreate()
        {
            if (!HasComponent<TransformComponent>())
            {
                Utils.Log("ObjController requires TransformComponent!");
                return;
            }

            m_Transform = GetComponent<TransformComponent>();
        }

        public void OnUpdate(float ts)
        {
            if (m_Transform == null)
                return;

            Vector3 translation = m_Transform.Translation;

            float moveSpeed = MoveSpeed * ts;

            // 切换控制开关
            bool currentQDown = Input.IsKeyDown(KeyCode.Q);
            if (currentQDown && !lastQDown)
            {
                enable = !enable;
            }
            lastQDown = currentQDown;

            if (enable)
            {
                // XYZ方向移动
                if (Input.IsKeyDown(KeyCode.W)) translation.Z += moveSpeed;
                if (Input.IsKeyDown(KeyCode.S)) translation.Z -= moveSpeed;
                if (Input.IsKeyDown(KeyCode.A)) translation.X -= moveSpeed;
                if (Input.IsKeyDown(KeyCode.D)) translation.X += moveSpeed;
                if (Input.IsKeyDown(KeyCode.Up)) translation.Y += moveSpeed;
                if (Input.IsKeyDown(KeyCode.Down)) translation.Y -= moveSpeed;
            }

            m_Transform.Translation = translation;
        }
    }
}
