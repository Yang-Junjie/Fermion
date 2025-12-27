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
    static AssetHandle CreateSphere(float radius = 0.5f, uint32_t latitudeSegments = 16, uint32_t longitudeSegments = 32);
    static void Init() {
        if (s_Initialized)
            return;

        s_BuiltinMeshes[MemoryMeshType::Cube] = CreateBox(glm::vec3(1));
        s_BuiltinMeshes[MemoryMeshType::Sphere] = CreateSphere(0.5f);
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
