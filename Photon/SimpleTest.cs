using System;
using Fermion;
using static Fermion.InternalCalls;
namespace Sandbox
{
    public class SimpleTest : Entity
    {
        public double Counter = 0;
        public double sum = 0;
        public int a = 1;
        // private BoxSensor2DComponent m_boxSensor2D;
        public void OnCreate()
        {
            // m_boxSensor2D = GetComponent<BoxSensor2DComponent>();
            ConsoleLog("[SimpleTest] create successfully !");
        }

        public void OnUpdate(float ts)
        {
            Counter++;
            if (Counter <= 100)
            {
                ConsoleLog($"[SimpleTest] Update #{Counter}, DeltaTime: {ts}");
                sum += ts;

            }

            if (a > 0 && Counter >= 100)
            {
                ConsoleLog($"[SimpleTest] average: {sum / Counter}");
                a--;
            }
            // if (m_boxSensor2D.SensorBegin)
            // {
            //     ConsoleLog("[SimpleTest] SensorBegin");
            // }
            // if (m_boxSensor2D.SensorEnd)
            // {
            //     ConsoleLog("[SimpleTest] SensorEnd");
            // }


        }
    }
}
