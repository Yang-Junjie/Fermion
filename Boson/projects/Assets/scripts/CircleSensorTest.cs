using System;
using Fermion;

namespace Photon
{
    public class CircleSensorTest : Entity
    {
        private CircleSensor2DComponent m_Sensor;

        private int m_ContactCount = 0;
        private bool IsTriggered => m_ContactCount > 0;

        public void OnCreate()
        {
            m_Sensor = AddComponent<CircleSensor2DComponent>();
            m_Sensor.Radius = 1.0f;
            m_Sensor.Offset = new Vector2(0.0f, 0.0f);

            Utils.Log("[CircleSensorTest] Created, radius=1.0");
        }

        public void OnUpdate(float ts)
        {
            if (m_Sensor.SensorBegin)
            {
                m_ContactCount++;
                Utils.Log("[CircleSensorTest] SensorBegin, contacts=" + m_ContactCount);
            }
            if (m_Sensor.SensorEnd)
            {
                m_ContactCount--;
                if (m_ContactCount < 0)
                    m_ContactCount = 0;
                Utils.Log("[CircleSensorTest] SensorEnd, contacts=" + m_ContactCount);
            }
        }
    }
}
