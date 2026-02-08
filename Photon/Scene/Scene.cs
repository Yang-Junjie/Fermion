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

        public static void DestroyEntity(Entity entity)
        {
            InternalCalls.Scene_DestroyEntity(entity.ID);
        }

        public static void InitPhysics3DEntity(Entity entity)
        {
            InternalCalls.Scene_InitPhysics3DEntity(entity.ID);
        }
    }


}
