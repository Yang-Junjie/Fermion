using System;
using Fermion;

namespace Photon
{
    public class CharacterController2D : Entity
    {
        // --- 可配置参数 ---
        public float MoveForce = 0.05f;
        public float MaxMoveSpeed = 8.0f;
        public float JumpImpulse = 15.0f;
        public float SprintMultiplier = 1.6f;

        // --- 组件引用 ---
        private Rigidbody2DComponent m_Rigidbody;
        private BoxSensor2DComponent m_GroundSensor;
        private TransformComponent m_Transform;

        // --- 地面检测 ---
        private int m_GroundContactCount = 0;
        private bool IsGrounded => m_GroundContactCount > 0;

        // 跳跃边缘检测
        private bool m_LastJumpDown = false;

        public void OnCreate()
        {
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
            m_Transform = GetComponent<TransformComponent>();

            // 脚底地面检测传感器
            m_GroundSensor = AddComponent<BoxSensor2DComponent>();
            m_GroundSensor.Size = new Vector2(0.4f, 0.1f);
            m_GroundSensor.Offset = new Vector2(0.0f, -0.55f);
        }

        public void OnUpdate(float ts)
        {
            UpdateGroundSensor();
            HandleMovement(ts);
            HandleJump();
            ClampVelocity();
            UpdateFacing();
        }

        private void UpdateGroundSensor()
        {
            if (m_GroundSensor.SensorBegin)
                m_GroundContactCount++;
            if (m_GroundSensor.SensorEnd)
                m_GroundContactCount--;
            if (m_GroundContactCount < 0)
                m_GroundContactCount = 0;
        }

        private void HandleMovement(float ts)
        {
            bool left = Input.IsKeyDown(KeyCode.A) || Input.IsKeyDown(KeyCode.Left);
            bool right = Input.IsKeyDown(KeyCode.D) || Input.IsKeyDown(KeyCode.Right);
            bool sprint = Input.IsKeyDown(KeyCode.LeftShift);

            float force = MoveForce;
            float maxSpd = MaxMoveSpeed;
            if (sprint)
            {
                force *= SprintMultiplier;
                maxSpd *= SprintMultiplier;
            }

            Vector2 vel = m_Rigidbody.LinearVelocity;

            if (left && vel.X > -maxSpd)
                m_Rigidbody.ApplyLinearImpulse(new Vector2(-force, 0.0f), true);
            else if (right && vel.X < maxSpd)
                m_Rigidbody.ApplyLinearImpulse(new Vector2(force, 0.0f), true);
        }

        private void HandleJump()
        {
            bool jumpDown = Input.IsKeyDown(KeyCode.Space) || Input.IsKeyDown(KeyCode.W);

            if (jumpDown && !m_LastJumpDown && IsGrounded)
                m_Rigidbody.ApplyLinearImpulse(new Vector2(0.0f, JumpImpulse), true);

            m_LastJumpDown = jumpDown;
        }

        private void ClampVelocity()
        {
        }

        private void UpdateFacing()
        {
            Vector2 vel = m_Rigidbody.LinearVelocity;
            Vector3 scale = m_Transform.Scale;

            if (vel.X > 0.1f)
                scale.X = Math.Abs(scale.X);
            else if (vel.X < -0.1f)
                scale.X = -Math.Abs(scale.X);

            m_Transform.Scale = scale;
        }
    }
}
