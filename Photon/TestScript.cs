using System;
using System.Runtime.CompilerServices;

public class TestScript
{
    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void PrintFromCpp();

    public static void OnCreate()
    {
        Console.WriteLine("Hello from C#!");
        PrintFromCpp();
    }
}
