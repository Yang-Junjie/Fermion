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

}
