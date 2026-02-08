using System;
using System.Runtime.CompilerServices;

namespace Fermion
{
    public static class InternalCalls
    {

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Scene_CreateEntity(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Scene_DestroyEntity(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Scene_InitPhysics3DEntity(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void NativeLog(string s, int parameter);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void ConsoleLog(string s);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern object GetScriptInstance(ulong entityID, string className);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Entity_HasComponent(ulong id, Type componentType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Entity_FindEntityByName(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_AddComponent(ulong id, Type componentType);



        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetRotation(ulong entityID, out Vector3 rotation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetRotation(ulong entityID, ref Vector3 rotation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetScale(ulong entityID, out Vector3 scale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetScale(ulong entityID, ref Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpriteRendererComponent_SetColor(ulong entityID, ref Vector4 translation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpriteRendererComponent_SetTexture(ulong entityID, ulong uuid);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static Rigidbody2DComponent.BodyType Rigidbody2DComponent_GetType(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_SetType(ulong entityID, Rigidbody2DComponent.BodyType type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse, bool wake);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_GetLinearVelocity(ulong entityID, out Vector2 linearVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_SetLinearVelocity(ulong entityID, ref Vector2 linearVelocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static Rigidbody3DComponent.BodyType Rigidbody3DComponent_GetType(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_SetType(ulong entityID, Rigidbody3DComponent.BodyType type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector3 impulse, bool wake);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_ApplyAngularImpulse(ulong entityID, ref Vector3 impulse, bool wake);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_AddForce(ulong entityID, ref Vector3 force, bool wake);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_GetLinearVelocity(ulong entityID, out Vector3 linearVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_SetLinearVelocity(ulong entityID, ref Vector3 linearVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_GetAngularVelocity(ulong entityID, out Vector3 angularVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_SetAngularVelocity(ulong entityID, ref Vector3 angularVelocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float Rigidbody3DComponent_GetMass(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_SetMass(ulong entityID, float mass);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Rigidbody3DComponent_GetUseGravity(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_SetUseGravity(ulong entityID, bool useGravity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Rigidbody3DComponent_GetFixedRotation(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_SetFixedRotation(ulong entityID, bool fixedRotation);

        // MeshComponent
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void MeshComponent_SetMemoryMesh(ulong entityID, int meshType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void MeshComponent_SetMaterialColor(ulong entityID, ref Vector3 color);

        // BoxCollider3DComponent
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider3DComponent_GetSize(ulong entityID, out Vector3 size);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider3DComponent_SetSize(ulong entityID, ref Vector3 size);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider3DComponent_GetOffset(ulong entityID, out Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider3DComponent_SetOffset(ulong entityID, ref Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float BoxCollider3DComponent_GetFriction(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider3DComponent_SetFriction(ulong entityID, float friction);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float BoxCollider3DComponent_GetRestitution(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider3DComponent_SetRestitution(ulong entityID, float restitution);

        // CircleCollider3DComponent
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float CircleCollider3DComponent_GetRadius(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider3DComponent_SetRadius(ulong entityID, float radius);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider3DComponent_GetOffset(ulong entityID, out Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider3DComponent_SetOffset(ulong entityID, ref Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float CircleCollider3DComponent_GetFriction(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider3DComponent_SetFriction(ulong entityID, float friction);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float CircleCollider3DComponent_GetRestitution(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider3DComponent_SetRestitution(ulong entityID, float restitution);

        // CapsuleCollider3DComponent
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float CapsuleCollider3DComponent_GetRadius(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void CapsuleCollider3DComponent_SetRadius(ulong entityID, float radius);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float CapsuleCollider3DComponent_GetHeight(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void CapsuleCollider3DComponent_SetHeight(ulong entityID, float height);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void CapsuleCollider3DComponent_GetOffset(ulong entityID, out Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void CapsuleCollider3DComponent_SetOffset(ulong entityID, ref Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float CapsuleCollider3DComponent_GetFriction(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void CapsuleCollider3DComponent_SetFriction(ulong entityID, float friction);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float CapsuleCollider3DComponent_GetRestitution(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void CapsuleCollider3DComponent_SetRestitution(ulong entityID, float restitution);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool BoxSensor2D_SensorBegin(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool BoxSensor2D_SensorEnd(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxSensor2D_SetSize(ulong entityID, ref Vector2 size);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxSensor2D_GetSize(ulong entityID, ref Vector2 size);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxSensor2D_SetOffset(ulong entityID, ref Vector2 size);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxSensor2D_GetOffset(ulong entityID, ref Vector2 size);






        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsKeyDown(KeyCode keycode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Input_SetCursorMode(CursorMode mode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Input_SetMousePosition(float x, float y);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Input_GetMousePosition(out float x, out float y);



        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static unsafe extern void DebugRenderer_DrawLine(Vector3* start, Vector3* end, Vector4* color);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static unsafe extern void DebugRenderer_SetLineWidth(float width);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static unsafe extern void DebugRenderer_DrawQuadBillboard(Vector3* position, Vector2* size, Vector4* color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_SetText(ulong entityID, string s);


    }
}
