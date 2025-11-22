using System;
using System.Runtime.CompilerServices;
using Fermion;
namespace Sandbox
{
    public class TestScript : Entity
    {
        public float MyFloatVar = 3.14f;
        public int MyIntVar = 42;
        public bool MyBoolVar = true;


        public void OnCreate()
        {
            Console.WriteLine("Hello from C#!");
            Console.WriteLine($"[TestScript] Entity ID = {ID}");
            InternalCalls.NativeLog("native log",123);
            Console.WriteLine($"[TestScript] About to call TransformComponent_GetTranslation with ID = {ID}");
            InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 translation);
            Console.WriteLine($"[TestScript] Translation: {translation.X},{translation.Y},{translation.Z}");
        }

        public void OnUpdate(float ts)
        {
            Console.WriteLine("on update: " + ts);
        }
    }
}