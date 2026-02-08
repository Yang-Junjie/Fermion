#include "MaterialFactory.hpp"
#include "Material.hpp"
#include "MaterialSerializer.hpp"
#include "Project/Project.hpp"
#include "Asset/AssetManager.hpp"

namespace Fermion
{
    AssetHandle MaterialFactory::createMemoryOnlyMaterial(const glm::vec3 &albedo, float metallic, float roughness, float AO)
    {
        auto material = std::make_shared<Material>();
        material->setMaterialType(MaterialType::PBR);
        material->setName("MemoryMaterial");
        material->setAlbedo(albedo);
        material->setMetallic(metallic);
        material->setRoughness(roughness);
        material->setAO(AO);

        return AssetManager::addMemoryOnlyAsset(material);
    }

    AssetHandle MaterialFactory::createMaterial(std::string name, const glm::vec3 &albedo, float metallic, float roughness, float AO)
    {

        auto material = std::make_shared<Material>();
        material->setMaterialType(MaterialType::PBR);
        material->setName(name);
        material->setAlbedo(albedo);
        material->setMetallic(metallic);
        material->setRoughness(roughness);
        material->setAO(AO);

        material->handle = AssetHandle();

        auto project = Project::getActive();
        if (!project)
        {
            return AssetHandle(0);
        }

        auto &config = project->getConfig();
        std::filesystem::path materialsDir = config.assetDirectory / "Materials";

        if (!std::filesystem::exists(materialsDir))
        {
            std::filesystem::create_directories(materialsDir);
        }

        std::string materialName = name;
        std::filesystem::path materialPath = materialsDir / (materialName + ".fmat");

        if (!MaterialSerializer::serialize(materialPath, *material))
            return AssetHandle(0);

        AssetHandle handle = AssetManager::importAsset(materialPath);
        
        material->handle = handle;
        AssetManager::registerLoadedAsset(handle, material);

        return handle;
    }
    AssetHandle MaterialFactory::createMaterial(std::string name, const glm::vec4 &DiffuseColor, const glm::vec4 &AmbientColor, AssetHandle TextureHandle)
    {
        auto material = std::make_shared<Material>();
        material->setMaterialType(MaterialType::Phong);
        material->setName(name);
        material->setDiffuseColor(DiffuseColor);
        material->setAmbientColor(AmbientColor);
        material->setDiffuseTexture(TextureHandle);

        material->handle = AssetHandle();

        auto project = Project::getActive();
        if (!project)
        {
            return AssetHandle(0);
        }

        auto &config = project->getConfig();
        std::filesystem::path materialsDir = config.assetDirectory / "Materials";

        if (!std::filesystem::exists(materialsDir))
        {
            std::filesystem::create_directories(materialsDir);
        }

        std::string materialName = name;
        std::filesystem::path materialPath = materialsDir / (materialName + ".fmat");

        if (!MaterialSerializer::serialize(materialPath, *material))
            return AssetHandle(0);

        AssetHandle handle = AssetManager::importAsset(materialPath);
        
        material->handle = handle;
        AssetManager::registerLoadedAsset(handle, material);

        return handle;
    }

    AssetHandle MaterialFactory::createMaterial(std::string name, const MapAssets &mapAssets,const glm::vec3 &albedo , float metallic, float roughness , float AO )
    {
        auto material = std::make_shared<Material>();
        material->setMaterialType(MaterialType::PBR);
        material->setName(name);
        
        // 设置默认的 PBR 数值参数
        material->setAlbedo(albedo);
        material->setMetallic(metallic);
        material->setRoughness(roughness);
        material->setAO(AO);

        // 设置贴图
        material->setAlbedoMap(mapAssets.AlbedoMapHandle);
        material->setNormalMap(mapAssets.NormalMapHandle);
        material->setMetallicMap(mapAssets.MetallicMapHandle);
        material->setRoughnessMap(mapAssets.RoughnessMapHandle);
        material->setAOMap(mapAssets.AOMapHandle);

        material->handle = AssetHandle();

        auto project = Project::getActive();
        if (!project)
        {
            return AssetHandle(0);
        }

        auto &config = project->getConfig();
        std::filesystem::path materialsDir = config.assetDirectory / "Materials";

        if (!std::filesystem::exists(materialsDir))
        {
            std::filesystem::create_directories(materialsDir);
        }

        std::string materialName = name;
        std::filesystem::path materialPath = materialsDir / (materialName + ".fmat");

        MaterialSerializeOptions options;
        options.includePBRMaps = true;
        if (!MaterialSerializer::serialize(materialPath, *material, options))
            return AssetHandle(0);

        AssetHandle handle = AssetManager::importAsset(materialPath);
        
        material->handle = handle;
        AssetManager::registerLoadedAsset(handle, material);

        return handle;
    }

    AssetHandle MaterialFactory::createMaterial(std::string name, const MapAssets &mapAssets, const glm::vec3 &albedo, float metallic, float roughness, float AO, const MaterialNodeEditorData &editorData)
    {
        auto material = std::make_shared<Material>();
        material->setMaterialType(MaterialType::PBR);
        material->setName(name);

        material->setAlbedo(albedo);
        material->setMetallic(metallic);
        material->setRoughness(roughness);
        material->setAO(AO);

        material->setAlbedoMap(mapAssets.AlbedoMapHandle);
        material->setNormalMap(mapAssets.NormalMapHandle);
        material->setMetallicMap(mapAssets.MetallicMapHandle);
        material->setRoughnessMap(mapAssets.RoughnessMapHandle);
        material->setAOMap(mapAssets.AOMapHandle);

        material->setEditorData(editorData);

        material->handle = AssetHandle();

        auto project = Project::getActive();
        if (!project)
        {
            return AssetHandle(0);
        }

        auto &config = project->getConfig();
        std::filesystem::path materialsDir = config.assetDirectory / "Materials";

        if (!std::filesystem::exists(materialsDir))
        {
            std::filesystem::create_directories(materialsDir);
        }

        std::string materialName = name;
        std::filesystem::path materialPath = materialsDir / (materialName + ".fmat");

        MaterialSerializeOptions options;
        options.includePBRMaps = true;
        options.includeEditorData = true;
        if (!MaterialSerializer::serialize(materialPath, *material, options))
            return AssetHandle(0);

        AssetHandle handle = AssetManager::importAsset(materialPath);

        material->handle = handle;
        AssetManager::registerLoadedAsset(handle, material);

        return handle;
    }
}
