#pragma once
#include "Asset/Asset.hpp"
#include <glm/glm.hpp>
#include <unordered_map>

#include "Mesh.hpp"

namespace Fermion
{
enum MemoryMeshType : uint16_t;

class MeshFactory {
public:
    static AssetHandle CreateBox(const glm::vec3 &size);

    static AssetHandle CreateSphere(float radius = 0.5f, uint32_t latitudeSegments = 16,
                                    uint32_t longitudeSegments = 32);

    static AssetHandle CreateCylinder(float radius = 0.5f, float height = 1.0f,
                                      uint32_t radialSegments = 32);

    static AssetHandle CreateCapsule(float radius = 0.5f, float height = 1.5f,
                                     uint32_t radialSegments = 32,
                                     uint32_t hemisphereSegments = 8);

    static AssetHandle CreateCone(float radius = 0.5f, float height = 1.0f,
                                  uint32_t radialSegments = 32);

    static void Init() {
        if (s_Initialized)
            return;

        s_BuiltinMeshes[MemoryMeshType::Cube] = CreateBox(glm::vec3(1));
        s_BuiltinMeshes[MemoryMeshType::Sphere] = CreateSphere(0.5f);
        s_BuiltinMeshes[MemoryMeshType::Cylinder] = CreateCylinder(0.5f, 1.0f, 32);
        s_BuiltinMeshes[MemoryMeshType::Capsule] = CreateCapsule(0.5f, 1.5f, 32, 8);
        s_BuiltinMeshes[MemoryMeshType::Cone] = CreateCone(0.5f, 1.0f, 32);

        s_Initialized = true;
    }

    static AssetHandle GetMemoryMeshHandle(MemoryMeshType type) {
        if (!s_Initialized)
            Init();

        if (auto it = s_BuiltinMeshes.find(type); it != s_BuiltinMeshes.end())
            return it->second;

        return AssetHandle(0);
    }

private:
    inline static bool s_Initialized = false;
    inline static std::unordered_map<MemoryMeshType, AssetHandle> s_BuiltinMeshes;
};
} // namespace Fermion
