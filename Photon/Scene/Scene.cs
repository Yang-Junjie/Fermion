using System;
using System.Runtime.CompilerServices;

namespace Fermion
{
    public class Scene
    {
        public static Entity CreateEntity(string tag = "Unnamed")
        {
            return new Entity(InternalCalls.Scene_CreateEntity(tag));
        }
    }


}
