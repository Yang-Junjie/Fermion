using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Fermion
{
	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class TransformComponent : Component
	{
		public Vector3 Translation
		{
			get
			{
				InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
				return translation;
			}
			set
			{
				InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
			}
		}

		public Vector3 Rotation
		{
			get
			{
				InternalCalls.TransformComponent_GetRotation(Entity.ID, out Vector3 rotation);
				return rotation;
			}
			set
			{
				InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value);
			}
		}

		public Vector3 Scale
		{
			get
			{
				InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 scale);
				return scale;
			}
			set
			{
				InternalCalls.TransformComponent_SetScale(Entity.ID, ref value);
			}
		}
	}

	public class SpriteRendererComponent : Component
	{
		public Vector4 Color
		{
			get
			{
				return new Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			}
			set
			{
				InternalCalls.SpriteRendererComponent_SetColor(Entity.ID, ref value);
			}
		}
		public void SetTexture(ulong uuid)
		{
			InternalCalls.SpriteRendererComponent_SetTexture(Entity.ID, uuid);
		}
	}
	public class Rigidbody2DComponent : Component
	{
		public enum BodyType { Static = 0, Dynamic = 1, Kinematic = 2 }

		public Vector2 LinearVelocity
		{
			get
			{
				InternalCalls.Rigidbody2DComponent_GetLinearVelocity(Entity.ID, out Vector2 velocity);
				return velocity;
			}
			set
			{
				InternalCalls.Rigidbody2DComponent_SetLinearVelocity(Entity.ID, ref value);
			}
		}

		public BodyType Type
		{
			get => InternalCalls.Rigidbody2DComponent_GetType(Entity.ID);
			set => InternalCalls.Rigidbody2DComponent_SetType(Entity.ID, value);
		}

		public void ApplyLinearImpulse(Vector2 impulse, bool wake)
		{
			InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
		}


	}
	public class Rigidbody3DComponent : Component
	{
		public enum BodyType { Static = 0, Dynamic = 1, Kinematic = 2 }

		public Vector3 LinearVelocity
		{
			get
			{
				InternalCalls.Rigidbody3DComponent_GetLinearVelocity(Entity.ID, out Vector3 velocity);
				return velocity;
			}
			set
			{
				InternalCalls.Rigidbody3DComponent_SetLinearVelocity(Entity.ID, ref value);
			}
		}

		public Vector3 AngularVelocity
		{
			get
			{
				InternalCalls.Rigidbody3DComponent_GetAngularVelocity(Entity.ID, out Vector3 velocity);
				return velocity;
			}
			set
			{
				InternalCalls.Rigidbody3DComponent_SetAngularVelocity(Entity.ID, ref value);
			}
		}

		public BodyType Type
		{
			get => InternalCalls.Rigidbody3DComponent_GetType(Entity.ID);
			set => InternalCalls.Rigidbody3DComponent_SetType(Entity.ID, value);
		}

		public float Mass
		{
			get => InternalCalls.Rigidbody3DComponent_GetMass(Entity.ID);
			set => InternalCalls.Rigidbody3DComponent_SetMass(Entity.ID, value);
		}

		public bool UseGravity
		{
			get => InternalCalls.Rigidbody3DComponent_GetUseGravity(Entity.ID);
			set => InternalCalls.Rigidbody3DComponent_SetUseGravity(Entity.ID, value);
		}

		public bool FixedRotation
		{
			get => InternalCalls.Rigidbody3DComponent_GetFixedRotation(Entity.ID);
			set => InternalCalls.Rigidbody3DComponent_SetFixedRotation(Entity.ID, value);
		}

		public void ApplyLinearImpulse(Vector3 impulse, bool wake)
		{
			InternalCalls.Rigidbody3DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
		}

		public void ApplyAngularImpulse(Vector3 impulse, bool wake)
		{
			InternalCalls.Rigidbody3DComponent_ApplyAngularImpulse(Entity.ID, ref impulse, wake);
		}

		public void AddForce(Vector3 force, bool wake)
		{
			InternalCalls.Rigidbody3DComponent_AddForce(Entity.ID, ref force, wake);
		}
	}
	public class BoxSensor2DComponent : Component
	{
		public Vector2 Size
		{
			get
			{
				Vector2 size = default(Vector2);
				InternalCalls.BoxSensor2D_GetSize(Entity.ID, ref size);
				return size;
			}
			set
			{
				InternalCalls.BoxSensor2D_SetSize(Entity.ID, ref value);
			}
		}

		public Vector2 Offset
		{
			get
			{
				Vector2 offset = default(Vector2);
				InternalCalls.BoxSensor2D_GetOffset(Entity.ID, ref offset);
				return offset;
			}
			set
			{
				InternalCalls.BoxSensor2D_SetOffset(Entity.ID, ref value);
			}
		}

		public bool SensorBegin
			=> InternalCalls.BoxSensor2D_SensorBegin(Entity.ID);

		public bool SensorEnd
			=> InternalCalls.BoxSensor2D_SensorEnd(Entity.ID);
	}

	public class CircleSensor2DComponent : Component
	{
		public float Radius
		{
			get => InternalCalls.CircleSensor2D_GetRadius(Entity.ID);
			set => InternalCalls.CircleSensor2D_SetRadius(Entity.ID, value);
		}

		public Vector2 Offset
		{
			get
			{
				Vector2 offset = default(Vector2);
				InternalCalls.CircleSensor2D_GetOffset(Entity.ID, ref offset);
				return offset;
			}
			set
			{
				InternalCalls.CircleSensor2D_SetOffset(Entity.ID, ref value);
			}
		}

		public bool SensorBegin
			=> InternalCalls.CircleSensor2D_SensorBegin(Entity.ID);

		public bool SensorEnd
			=> InternalCalls.CircleSensor2D_SensorEnd(Entity.ID);
	}

	public class TextComponent : Component
	{
		public string Text
		{
			set
			{
				InternalCalls.TextComponent_SetText(Entity.ID, value);
			}
		}
	}

	public enum MemoryMeshType
	{
		None = 0,
		Cube = 1,
		Sphere = 2,
		Cylinder = 3,
		Capsule = 4,
		Cone = 5
	}

	public class MeshComponent : Component
	{
		public void SetMemoryMesh(MemoryMeshType meshType)
		{
			InternalCalls.MeshComponent_SetMemoryMesh(Entity.ID, (int)meshType);
		}

		public void SetMaterialColor(Vector3 color)
		{
			InternalCalls.MeshComponent_SetMaterialColor(Entity.ID, ref color);
		}
	}

	public class BoxCollider3DComponent : Component
	{
		public Vector3 Size
		{
			get
			{
				InternalCalls.BoxCollider3DComponent_GetSize(Entity.ID, out Vector3 size);
				return size;
			}
			set
			{
				InternalCalls.BoxCollider3DComponent_SetSize(Entity.ID, ref value);
			}
		}

		public Vector3 Offset
		{
			get
			{
				InternalCalls.BoxCollider3DComponent_GetOffset(Entity.ID, out Vector3 offset);
				return offset;
			}
			set
			{
				InternalCalls.BoxCollider3DComponent_SetOffset(Entity.ID, ref value);
			}
		}

		public float Friction
		{
			get => InternalCalls.BoxCollider3DComponent_GetFriction(Entity.ID);
			set => InternalCalls.BoxCollider3DComponent_SetFriction(Entity.ID, value);
		}

		public float Restitution
		{
			get => InternalCalls.BoxCollider3DComponent_GetRestitution(Entity.ID);
			set => InternalCalls.BoxCollider3DComponent_SetRestitution(Entity.ID, value);
		}
	}

	public class CircleCollider3DComponent : Component
	{
		public float Radius
		{
			get => InternalCalls.CircleCollider3DComponent_GetRadius(Entity.ID);
			set => InternalCalls.CircleCollider3DComponent_SetRadius(Entity.ID, value);
		}

		public Vector3 Offset
		{
			get
			{
				InternalCalls.CircleCollider3DComponent_GetOffset(Entity.ID, out Vector3 offset);
				return offset;
			}
			set
			{
				InternalCalls.CircleCollider3DComponent_SetOffset(Entity.ID, ref value);
			}
		}

		public float Friction
		{
			get => InternalCalls.CircleCollider3DComponent_GetFriction(Entity.ID);
			set => InternalCalls.CircleCollider3DComponent_SetFriction(Entity.ID, value);
		}

		public float Restitution
		{
			get => InternalCalls.CircleCollider3DComponent_GetRestitution(Entity.ID);
			set => InternalCalls.CircleCollider3DComponent_SetRestitution(Entity.ID, value);
		}
	}

	public class CapsuleCollider3DComponent : Component
	{
		public float Radius
		{
			get => InternalCalls.CapsuleCollider3DComponent_GetRadius(Entity.ID);
			set => InternalCalls.CapsuleCollider3DComponent_SetRadius(Entity.ID, value);
		}

		public float Height
		{
			get => InternalCalls.CapsuleCollider3DComponent_GetHeight(Entity.ID);
			set => InternalCalls.CapsuleCollider3DComponent_SetHeight(Entity.ID, value);
		}

		public Vector3 Offset
		{
			get
			{
				InternalCalls.CapsuleCollider3DComponent_GetOffset(Entity.ID, out Vector3 offset);
				return offset;
			}
			set
			{
				InternalCalls.CapsuleCollider3DComponent_SetOffset(Entity.ID, ref value);
			}
		}

		public float Friction
		{
			get => InternalCalls.CapsuleCollider3DComponent_GetFriction(Entity.ID);
			set => InternalCalls.CapsuleCollider3DComponent_SetFriction(Entity.ID, value);
		}

		public float Restitution
		{
			get => InternalCalls.CapsuleCollider3DComponent_GetRestitution(Entity.ID);
			set => InternalCalls.CapsuleCollider3DComponent_SetRestitution(Entity.ID, value);
		}
	}

	public class RevoluteJoint2DComponent : Component
	{
	}

}
