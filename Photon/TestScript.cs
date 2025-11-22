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

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void PrintFromCpp();

        public static void OnCreate()
        {
            Console.WriteLine("Hello from C#!");
            PrintFromCpp();
        }

        public void OnUpdate(float ts)
        {
            Console.WriteLine("on update: " + ts);
        }
    }
}