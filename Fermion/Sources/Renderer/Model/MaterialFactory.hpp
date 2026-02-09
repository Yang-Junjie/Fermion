#pragma once
#include "Asset/Asset.hpp"
#include "Material.hpp"
#include <glm/glm.hpp>

namespace Fermion
{
    class MaterialFactory
    {
    public:
        // create a default material
        static AssetHandle createMaterial(std::string name = "Material", const glm::vec3 &albedo = glm::vec3(1.0f), float metallic = 0.0f, float roughness = 0.5f, float AO = 1.0f);

        // create a memory-only material 
        static AssetHandle createMemoryOnlyMaterial(const glm::vec3 &albedo = glm::vec3(1.0f), float metallic = 0.0f, float roughness = 0.5f, float AO = 1.0f);

        static AssetHandle createMaterial(std::string name, const glm::vec4 &DiffuseColor, const glm::vec4 &AmbientColor, AssetHandle TextureHandle);

        static AssetHandle createMaterial(std::string name, const MapAssets &mapAssets, const glm::vec3 &albedo = glm::vec3(1.0f), float metallic = 0.0f, float roughness = 0.5f, float AO = 1.0f);

        static AssetHandle createMaterial(std::string name, const MapAssets &mapAssets, const glm::vec3 &albedo, float metallic, float roughness, float AO, const MaterialNodeEditorData &editorData);
    };
}