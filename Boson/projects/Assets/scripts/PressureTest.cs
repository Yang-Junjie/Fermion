using System;
using Fermion;

namespace Sandbox
{
    public class PressureTest : Entity
    {
        public void OnCreate()
        {
            float width = 200.0f;
            float height = 1000.0f;

            for (float j = 0; j < width; j++)
            {
                for (float i = 0; i < height; i++)
                {
                    Entity testEntity = Scene.CreateEntity($"TestEntity{i}{j}");
                    testEntity.GetComponent<TransformComponent>().Translation = new Vector3(i, j, 0);

                    SpriteRendererComponent sprite = testEntity.AddComponent<SpriteRendererComponent>();

                    sprite.Color = new Vector4(i / height, j / width, 0, 1);
                    sprite.SetTexture(10348572890839182050);
                }
            }
        }

        public void OnUpdate(float ts)
        {

        }
    }
}
