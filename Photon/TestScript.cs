using System;
using System.Runtime.CompilerServices;
using Fermion;
using static Fermion.InternalCalls;
namespace Sandbox
{
    public class TestScript : Entity
    {
        float speed = 5.0f;
        public void OnCreate()
        {
            Console.WriteLine("Hello from C#!");
            Console.WriteLine($"[TestScript] Entity ID = {ID}");

            // InternalCalls.NativeLog("native log", 123);
            // Console.WriteLine($"[TestScript] About to call TransformComponent_GetTranslation with ID = {ID}");
            // InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 translation);
            Console.WriteLine($"[TestScript] Translation: {Translation.X},{Translation.Y},{Translation.Z}");


        }

        public void OnUpdate(float ts)
        {
            // Console.WriteLine("on update: " + ts);
            // InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 translationOrigin);
            // Vector3 translation = translationOrigin;
            // translation.X += ts;
            // InternalCalls.TransformComponent_SetTranslation(ID, ref translation);

            Vector3 pos = Translation;

            if (Input.IsKeyDown(KeyCode.W))
            {
                pos.Y += ts * speed;
                ConsoleLog("W"+$"[TestScript] Translation: {Translation.X},{Translation.Y},{Translation.Z}");
            }
            else if (Input.IsKeyDown(KeyCode.S))
            {
                pos.Y -= ts * speed;
                ConsoleLog("S"+$"[TestScript] Translation: {Translation.X},{Translation.Y},{Translation.Z}");
            }

            if (Input.IsKeyDown(KeyCode.A))
            {
                pos.X -= ts * speed;
                ConsoleLog("A"+$"[TestScript] Translation: {Translation.X},{Translation.Y},{Translation.Z}");
            }
            else if (Input.IsKeyDown(KeyCode.D))
            {
                pos.X += ts * speed;
                ConsoleLog("D"+$"[TestScript] Translation: {Translation.X},{Translation.Y},{Translation.Z}");
            }

            Translation = pos;

        }
    }
}