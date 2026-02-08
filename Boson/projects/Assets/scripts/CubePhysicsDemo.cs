using System;
using Fermion;

namespace Photon
{
    public class CubePhysicsDemo : Entity
    {
        public float CubeSize = 1.0f;
        public float Spacing = 1.2f;
        public float StartHeight = 10.0f;

        public void OnCreate()
        {
            CreateCubeGrid();
        }

        public void OnUpdate(float ts)
        {
        }

       
        private void CreateCubeGrid()
        {
            Random rand = new Random();
            float halfGrid = (5 - 1) * Spacing * 0.5f;

            for (int x = 0; x < 5; x++)
            {
                for (int y = 0; y < 5; y++)
                {
                    for (int z = 0; z < 5; z++)
                    {
                        float px = x * Spacing - halfGrid;
                        float py = y * Spacing + StartHeight;
                        float pz = z * Spacing - halfGrid;

                        // 每个方块随机颜色
                        float r = 0.3f + (float)rand.NextDouble() * 0.7f;
                        float g = 0.3f + (float)rand.NextDouble() * 0.7f;
                        float b = 0.3f + (float)rand.NextDouble() * 0.7f;

                        CreatePhysicsCube(
                            $"Cube_{x}_{y}_{z}",
                            new Vector3(px, py, pz),
                            new Vector3(r, g, b)
                        );
                    }
                }
            }
        }

        private void CreatePhysicsCube(string name, Vector3 position, Vector3 color)
        {
            Entity cube = Scene.CreateEntity(name);

            TransformComponent t = cube.GetComponent<TransformComponent>();
            t.Translation = position;
            t.Scale = new Vector3(CubeSize, CubeSize, CubeSize);

            MeshComponent mesh = cube.AddComponent<MeshComponent>();
            mesh.SetMemoryMesh(MemoryMeshType.Cube);
            mesh.SetMaterialColor(color);

            Rigidbody3DComponent rb = cube.AddComponent<Rigidbody3DComponent>();
            rb.Type = Rigidbody3DComponent.BodyType.Dynamic;
            rb.Mass = 1.0f;
            rb.UseGravity = true;

            BoxCollider3DComponent col = cube.AddComponent<BoxCollider3DComponent>();
            col.Size = new Vector3(0.5f, 0.5f, 0.5f);
            col.Friction = 0.5f;
            col.Restitution = 0.2f;

            Scene.InitPhysics3DEntity(cube);
        }
    }
}
