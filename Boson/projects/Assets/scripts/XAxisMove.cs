using Fermion;

namespace Sandbox
{
    public class XAxisMove : Entity
    {
        public float Speed = 5.0f;

        private TransformComponent m_Transform;
        private float direction = 1.0f;

        private float fixedY;
        private float fixedZ;

        public void OnCreate()
        {
            if (!HasComponent<TransformComponent>())
                return;

            m_Transform = GetComponent<TransformComponent>();

            fixedY = m_Transform.Translation.Y;
            fixedZ = m_Transform.Translation.Z;
        }

        public void OnUpdate(float ts)
        {
            if (m_Transform == null)
                return;

            Vector3 pos = m_Transform.Translation;

            pos.X += direction * Speed * ts;

            const float maxX = 5.0f;
            if (pos.X > maxX)
            {
                pos.X = maxX;
                direction = -1.0f;
            }
            else if (pos.X < -maxX)
            {
                pos.X = -maxX;
                direction = 1.0f;
            }

            pos.Y = fixedY;
            pos.Z = fixedZ;

            m_Transform.Translation = pos;
        }
    }
}
