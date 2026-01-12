#include "MaterialFactory.hpp"
#include "Material.hpp"
#include "Project/Project.hpp"
#include "Asset/AssetSerializer.hpp"
#include "Asset/AssetRegistry.hpp"
#include "Asset/AssetManager.hpp"
#include "Asset/AssetMetadata.hpp"
#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Fermion
{
    namespace
    {
        void serializeVec3(YAML::Emitter &out, const glm::vec3 &v)
        {
            out << YAML::Flow;
            out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        }

        void serializeVec4(YAML::Emitter &out, const glm::vec4 &v)
        {
            out << YAML::Flow;
            out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        }
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

        std::string materialName = "Material_" + name;
        std::filesystem::path materialPath = materialsDir / (materialName + ".fmat");

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Material" << YAML::Value;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Type" << YAML::Value << static_cast<int>(material->getType());
            out << YAML::Key << "Name" << YAML::Value << material->getName();

            out << YAML::Key << "Albedo" << YAML::Value;
            serializeVec3(out, material->getAlbedo());
            out << YAML::Key << "Metallic" << YAML::Value << material->getMetallic();
            out << YAML::Key << "Roughness" << YAML::Value << material->getRoughness();
            out << YAML::Key << "AO" << YAML::Value << material->getAO();

            out << YAML::EndMap;
        }
        out << YAML::EndMap;

        std::ofstream fout(materialPath);
        fout << out.c_str();
        fout.close();

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

        std::string materialName = "Material_" + name;
        std::filesystem::path materialPath = materialsDir / (materialName + ".fmat");

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Material" << YAML::Value;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Type" << YAML::Value << static_cast<int>(MaterialType::Phong);
            out << YAML::Key << "Name" << YAML::Value << material->getName();

            out << YAML::Key << "DiffuseColor" << YAML::Value;
            serializeVec4(out, material->getDiffuseColor());
            out << YAML::Key << "AmbientColor" << YAML::Value;
            serializeVec4(out, material->getAmbientColor());
            out << YAML::Key << "DiffuseTexture" << YAML::Value << static_cast<uint64_t>(material->getDiffuseTexture());

            out << YAML::EndMap;
        }
        out << YAML::EndMap;

        std::ofstream fout(materialPath);
        fout << out.c_str();
        fout.close();

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

        std::string materialName = "Material_" + name;
        std::filesystem::path materialPath = materialsDir / (materialName + ".fmat");

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Material" << YAML::Value;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Type" << YAML::Value << static_cast<int>(MaterialType::PBR);
            out << YAML::Key << "Name" << YAML::Value << material->getName();

            out << YAML::Key << "Albedo" << YAML::Value;
            serializeVec3(out, material->getAlbedo());
            out << YAML::Key << "Metallic" << YAML::Value << material->getMetallic();
            out << YAML::Key << "Roughness" << YAML::Value << material->getRoughness();
            out << YAML::Key << "AO" << YAML::Value << material->getAO();

            // 保存贴图引用
            out << YAML::Key << "AlbedoMap" << YAML::Value << static_cast<uint64_t>(material->getAlbedoMap());
            out << YAML::Key << "NormalMap" << YAML::Value << static_cast<uint64_t>(material->getNormalMap());
            out << YAML::Key << "MetallicMap" << YAML::Value << static_cast<uint64_t>(material->getMetallicMap());
            out << YAML::Key << "RoughnessMap" << YAML::Value << static_cast<uint64_t>(material->getRoughnessMap());
            out << YAML::Key << "AOMap" << YAML::Value << static_cast<uint64_t>(material->getAOMap());

            out << YAML::EndMap;
        }
        out << YAML::EndMap;

        std::ofstream fout(materialPath);
        fout << out.c_str();
        fout.close();

        AssetHandle handle = AssetManager::importAsset(materialPath);
        
        material->handle = handle;
        AssetManager::registerLoadedAsset(handle, material);

        return handle;
    }
}