using System;
using Fermion;

namespace Sandbox
{
    public class FPSCharacterController : Entity
    {
        public float MoveSpeed = 6.0f;
        public float SprintMultiplier = 2.0f;
        public float Acceleration = 30.0f;
        public float Deceleration = 40.0f;
        public bool FlyMode = false;
        public bool UseRigidbody = true;

        public float JumpForce = 8.0f;              
        public float GroundCheckThreshold = 0.1f;  
        public bool AllowAirControl = true;       

        public float MouseSensitivity = 0.002f;
        public float PitchMin = -1.5f; 
        public float PitchMax = 1.5f;   
        public bool LockCursorOnStart = true;

        public bool Enabled = true;
        public bool ToggleWithQ = true;
        public KeyCode ToggleCursorKey = KeyCode.Escape;
        public KeyCode FlyModeToggleKey = KeyCode.F;

        private TransformComponent m_Transform;
        private Rigidbody3DComponent m_Rigidbody;

        private Vector2 m_LastMousePosition;
        private bool m_CursorLocked = false;
        private bool m_FirstUpdate = true;
        private bool m_LastToggleCursorKeyDown = false;
        private bool m_LastToggleEnabledKeyDown = false;
        private bool m_LastFlyToggleDown = false;

        private bool m_IsGrounded = true;
        private bool m_JumpRequested = false;

        private float m_Pitch = 0.0f;
        private float m_Yaw = 0.0f;

        public void OnCreate()
        {
            if (!HasComponent<TransformComponent>())
            {
                Utils.Log("FPSCharacterController requires TransformComponent!");
                return;
            }

            m_Transform = GetComponent<TransformComponent>();

            Vector3 currentRotation = m_Transform.Rotation;
            m_Pitch = currentRotation.X;
            m_Yaw = currentRotation.Y;

            if (HasComponent<Rigidbody3DComponent>())
                m_Rigidbody = GetComponent<Rigidbody3DComponent>();
            else
                Utils.Log("FPSCharacterController: no Rigidbody3DComponent, using Transform only.");

            if (LockCursorOnStart)
            {
                LockCursor();
            }
        }

        public void OnUpdate(float ts)
        {
            if (m_Transform == null)
                return;

            if (ToggleWithQ)
            {
                bool toggleEnabledDown = Input.IsKeyDown(KeyCode.Q);
                if (toggleEnabledDown && !m_LastToggleEnabledKeyDown)
                    Enabled = !Enabled;
                m_LastToggleEnabledKeyDown = toggleEnabledDown;
            }

            bool flyToggleDown = Input.IsKeyDown(FlyModeToggleKey);
            if (flyToggleDown && !m_LastFlyToggleDown)
                FlyMode = !FlyMode;
            m_LastFlyToggleDown = flyToggleDown;

            bool toggleCursorDown = Input.IsKeyDown(ToggleCursorKey);
            if (toggleCursorDown && !m_LastToggleCursorKeyDown)
            {
                if (m_CursorLocked)
                    UnlockCursor();
                else
                    LockCursor();
            }
            m_LastToggleCursorKeyDown = toggleCursorDown;

            if (!Enabled)
                return;

            if (m_CursorLocked)
            {
                UpdateCameraRotation();
            }

            UpdateMovement(ts);
        }

        private void UpdateCameraRotation()
        {
            Vector2 currentMousePosition = Input.GetMousePosition();

            if (m_FirstUpdate)
            {
                m_LastMousePosition = currentMousePosition;
                m_FirstUpdate = false;
                return;
            }

            float deltaX = currentMousePosition.X - m_LastMousePosition.X;
            float deltaY = currentMousePosition.Y - m_LastMousePosition.Y;

            m_Yaw += deltaX * MouseSensitivity;

            m_Pitch -= deltaY * MouseSensitivity;
            m_Pitch = Clamp(m_Pitch, PitchMin, PitchMax);

            m_Transform.Rotation = new Vector3(m_Pitch, m_Yaw, 0.0f);

            m_LastMousePosition = currentMousePosition;
        }

        private void UpdateMovement(float ts)
        {
            if (UseRigidbody && m_Rigidbody != null && m_Rigidbody.Type == Rigidbody3DComponent.BodyType.Dynamic)
            {
                Vector3 velocity = m_Rigidbody.LinearVelocity;
                m_IsGrounded = Math.Abs(velocity.Y) < GroundCheckThreshold;
            }
            else
            {
                m_IsGrounded = true; 
            }

            Vector3 inputLocal = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W)) inputLocal.Z -= 1.0f;
            if (Input.IsKeyDown(KeyCode.S)) inputLocal.Z += 1.0f;
            if (Input.IsKeyDown(KeyCode.A)) inputLocal.X -= 1.0f;
            if (Input.IsKeyDown(KeyCode.D)) inputLocal.X += 1.0f;

            if (FlyMode)
            {
                if (Input.IsKeyDown(KeyCode.Space)) inputLocal.Y += 1.0f;
                if (Input.IsKeyDown(KeyCode.LeftControl) || Input.IsKeyDown(KeyCode.RightControl))
                    inputLocal.Y -= 1.0f;
            }
            else
            {
                if (Input.IsKeyDown(KeyCode.Space) && m_IsGrounded)
                {
                    m_JumpRequested = true;
                }
            }

            float lengthSq = inputLocal.X * inputLocal.X + inputLocal.Y * inputLocal.Y + inputLocal.Z * inputLocal.Z;
            if (lengthSq > 0.000001f)
            {
                float invLen = 1.0f / (float)Math.Sqrt(lengthSq);
                inputLocal = new Vector3(inputLocal.X * invLen, inputLocal.Y * invLen, inputLocal.Z * invLen);
            }

          
            Vector3 inputWorld = RotateVectorByYaw(inputLocal, m_Yaw);

            float speed = MoveSpeed;
            if (Input.IsKeyDown(KeyCode.LeftShift) || Input.IsKeyDown(KeyCode.RightShift))
                speed *= SprintMultiplier;

            Vector3 targetVelocity = new Vector3(inputWorld.X * speed, inputWorld.Y * speed, inputWorld.Z * speed);

            if (UseRigidbody && m_Rigidbody != null)
            {
                ApplyRigidbodyMovement(targetVelocity, lengthSq, ts);
            }
            else
            {
                ApplyTransformMovement(targetVelocity, ts);
            }
        }

        private void ApplyRigidbodyMovement(Vector3 targetVelocity, float inputLengthSq, float ts)
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
                float accel = inputLengthSq > 0.0f ? Acceleration : Deceleration;
                float maxDelta = accel * ts;

                Vector3 newVelocity = currentVelocity;

                if (FlyMode || AllowAirControl || m_IsGrounded)
                {
                    newVelocity.X = MoveTowards(currentVelocity.X, targetVelocity.X, maxDelta);
                    newVelocity.Z = MoveTowards(currentVelocity.Z, targetVelocity.Z, maxDelta);
                }

                if (FlyMode)
                {
                    newVelocity.Y = MoveTowards(currentVelocity.Y, targetVelocity.Y, maxDelta);
                }
                else
                {
                    newVelocity.Y = currentVelocity.Y;

                    if (m_JumpRequested && m_IsGrounded)
                    {
                        newVelocity.Y = JumpForce;
                        m_JumpRequested = false;
                    }
                }

                m_Rigidbody.LinearVelocity = newVelocity;
                return;
            }
        }

        private void ApplyTransformMovement(Vector3 targetVelocity, float ts)
        {
            Vector3 move = targetVelocity * ts;
            if (!FlyMode)
                move.Y = 0.0f;

            Vector3 translation = m_Transform.Translation;
            translation += move;
            m_Transform.Translation = translation;
        }

        private Vector3 RotateVectorByYaw(Vector3 vector, float yaw)
        {
            float cosYaw = (float)Math.Cos(yaw);
            float sinYaw = (float)Math.Sin(yaw);

            return new Vector3(
                vector.X * cosYaw - vector.Z * sinYaw,
                vector.Y,
                vector.X * sinYaw + vector.Z * cosYaw
            );
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
