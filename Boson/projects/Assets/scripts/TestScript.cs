using System;
using Fermion;

namespace Sandbox
{
    public class TestScript : Entity
    {
        public float MoveSpeed = 1.0f;
        public float JumpImpulse = 5.0f;
        public float Time = 0.0f;

        private Rigidbody2DComponent m_Rigidbody;
        private BoxSensor2DComponent m_Sensor;

        private int groundContactCount = 0;
        public bool IsGrounded => groundContactCount > 0;

        public void OnCreate()
        {
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
            m_Sensor = AddComponent<BoxSensor2DComponent>();

            m_Sensor.Size = new Vector2(0.5f, 0.1f);
            m_Sensor.Offset = new Vector2(0, -0.55f);
        }

        public void OnUpdate(float ts)
        {
            Time += ts;

            if (m_Sensor.SensorBegin)
                groundContactCount++;

            if (m_Sensor.SensorEnd)
                groundContactCount--;

            if (groundContactCount < 0)
                groundContactCount = 0;

            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.X = -0.1f;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.X = 0.1f;

            if (IsGrounded && Input.IsKeyDown(KeyCode.W))
            {
                m_Rigidbody.ApplyLinearImpulse(new Vector2(0, JumpImpulse), true);
            }

            m_Rigidbody.ApplyLinearImpulse(velocity.XY, true);

            velocity *= MoveSpeed * ts;
        }
    }
}
