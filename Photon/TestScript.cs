using System;
using System.Runtime.CompilerServices;
using Fermion;
using static Fermion.InternalCalls;
namespace Sandbox
{
    public class TestScript : Entity
    {
        float Speed = 50.0f;
        private Rigidbody2DComponent m_Rigidbody;
        public float Time = 0.0f;
        public void OnCreate()
        {
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
            string type;
            if (m_Rigidbody.Type == Rigidbody2DComponent.BodyType.Static)
            {
                type = "Static";
            }
            else if (m_Rigidbody.Type == Rigidbody2DComponent.BodyType.Kinematic)
            {
                type = "Kinematic";
            }
            else
            {
                type = "Dynamic";
            }
            ConsoleLog($"[TestScript] Rigidbody2DComponent.Type: {type}");

        }

        public void OnUpdate(float ts)
        {
            Time += ts;

            float speed = Speed;
            Vector3 velocity = Vector3.Zero;

            // ConsoleLog($"dt= {ts}");
            if (Input.IsKeyDown(KeyCode.W))
            {
                velocity.Y = 1.0f;
            }
            else if (Input.IsKeyDown(KeyCode.S))
            {
                velocity.Y = -1.0f;
            }

            if (Input.IsKeyDown(KeyCode.A))
            {
                velocity.X = -1.0f;
            }
            else if (Input.IsKeyDown(KeyCode.D))
            {
                velocity.X = 1.0f;
            }


            velocity *= speed * ts;

            m_Rigidbody.ApplyLinearImpulse(velocity.XY, true);
        }
    }
}