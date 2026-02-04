using System;
using Fermion;

namespace Photon
{
    public class Rigidbody3DController : Entity
    {
        public float MoveSpeed = 6.0f;
        public float SprintMultiplier = 2.0f;
        public float Acceleration = 30.0f;
        public float Deceleration = 40.0f;
        public bool FlyMode = false;
        public bool ToggleWithQ = true;
        public bool UseRigidbody = true;

        private TransformComponent m_Transform;
        private Rigidbody3DComponent m_Rigidbody;

        private bool m_Enabled = true;
        private bool m_LastToggleDown = false;
        private bool m_LastFlyToggleDown = false;

        public void OnCreate()
        {
            if (!HasComponent<TransformComponent>())
            {
                Utils.Log("Rigidbody3DController requires TransformComponent!");
                return;
            }

            m_Transform = GetComponent<TransformComponent>();

            if (HasComponent<Rigidbody3DComponent>())
                m_Rigidbody = GetComponent<Rigidbody3DComponent>();
            else
                Utils.Log("Rigidbody3DController: no Rigidbody3DComponent, using Transform only.");
        }

        public void OnUpdate(float ts)
        {
            if (m_Transform == null)
                return;

            if (ToggleWithQ)
            {
                bool toggleDown = Input.IsKeyDown(KeyCode.Q);
                if (toggleDown && !m_LastToggleDown)
                    m_Enabled = !m_Enabled;
                m_LastToggleDown = toggleDown;
            }

            bool flyToggleDown = Input.IsKeyDown(KeyCode.F);
            if (flyToggleDown && !m_LastFlyToggleDown)
                FlyMode = !FlyMode;
            m_LastFlyToggleDown = flyToggleDown;

            if (!m_Enabled)
                return;

            Vector3 input = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W)) input.Z -= 1.0f;
            if (Input.IsKeyDown(KeyCode.S)) input.Z += 1.0f;
            if (Input.IsKeyDown(KeyCode.A)) input.X -= 1.0f;
            if (Input.IsKeyDown(KeyCode.D)) input.X += 1.0f;

            if (FlyMode)
            {
                if (Input.IsKeyDown(KeyCode.Space)) input.Y += 1.0f;
                if (Input.IsKeyDown(KeyCode.LeftControl) || Input.IsKeyDown(KeyCode.RightControl)) input.Y -= 1.0f;
            }

            float lengthSq = input.X * input.X + input.Y * input.Y + input.Z * input.Z;
            if (lengthSq > 0.000001f)
            {
                float invLen = 1.0f / (float)Math.Sqrt(lengthSq);
                input = new Vector3(input.X * invLen, input.Y * invLen, input.Z * invLen);
            }

            float speed = MoveSpeed;
            if (Input.IsKeyDown(KeyCode.LeftShift) || Input.IsKeyDown(KeyCode.RightShift))
                speed *= SprintMultiplier;

            Vector3 targetVelocity = new Vector3(input.X * speed, input.Y * speed, input.Z * speed);

            if (UseRigidbody && m_Rigidbody != null)
            {
                if (m_Rigidbody.Type == Rigidbody3DComponent.BodyType.Kinematic)
                {
                    Vector3 translation = m_Transform.Translation;
                    translation += targetVelocity * ts;
                    m_Transform.Translation = translation;
                    return;
                }

                if (m_Rigidbody.Type == Rigidbody3DComponent.BodyType.Dynamic)
                {
                    Vector3 currentVelocity = m_Rigidbody.LinearVelocity;
                    float accel = lengthSq > 0.0f ? Acceleration : Deceleration;
                    float maxDelta = accel * ts;

                    Vector3 newVelocity = currentVelocity;
                    newVelocity.X = MoveTowards(currentVelocity.X, targetVelocity.X, maxDelta);
                    newVelocity.Z = MoveTowards(currentVelocity.Z, targetVelocity.Z, maxDelta);

                    if (FlyMode)
                        newVelocity.Y = MoveTowards(currentVelocity.Y, targetVelocity.Y, maxDelta);
                    else
                        newVelocity.Y = currentVelocity.Y;

                    m_Rigidbody.LinearVelocity = newVelocity;
                    return;
                }
            }

            Vector3 fallbackMove = targetVelocity * ts;
            if (!FlyMode)
                fallbackMove.Y = 0.0f;

            Vector3 fallbackTranslation = m_Transform.Translation;
            fallbackTranslation += fallbackMove;
            m_Transform.Translation = fallbackTranslation;
        }

        private static float MoveTowards(float current, float target, float maxDelta)
        {
            if (current < target)
                return Math.Min(current + maxDelta, target);
            if (current > target)
                return Math.Max(current - maxDelta, target);
            return target;
        }
    }
}
