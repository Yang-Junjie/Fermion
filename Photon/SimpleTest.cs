using System;
using Fermion;

namespace Sandbox
{
    public class SimpleTest : Entity
    {
        public int Counter = 0;
        
        public void OnCreate()
        {
            Console.WriteLine("[SimpleTest] 创建成功 - 脚本系统正常工作!");
        }
        
        public void OnUpdate(float ts)
        {
            Counter++;
            if (Counter <= 3)
            {
                Console.WriteLine($"[SimpleTest] Update #{Counter}, DeltaTime: {ts}");
            }
        }
    }
}
