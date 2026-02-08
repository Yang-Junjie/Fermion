using Fermion;

namespace Photon
{
    public class XAxisMove2D : Entity
    {
        public float Speed = 3.0f;
        public float BoundaryX = 5.0f;

        private Rigidbody2DComponent m_Rigidbody;
        private float m_Direction = 1.0f;

        public void OnCreate()
        {
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
        }

        public void OnUpdate(float ts)
        {
            if (m_Rigidbody == null)
                return;

            Vector3 pos = Translation;

            if (pos.X >= BoundaryX)
                m_Direction = -1.0f;
            else if (pos.X <= -BoundaryX)
                m_Direction = 1.0f;

            m_Rigidbody.LinearVelocity = new Vector2(m_Direction * Speed, 0.0f);
        }
    }
}
