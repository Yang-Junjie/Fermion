#include "MaterialLoader.hpp"
#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Fermion {

std::shared_ptr<Asset> MaterialLoader::load(const AssetMetadata &metadata) {
    if (!std::filesystem::exists(metadata.FilePath)) {
        return nullptr;
    }
    
    try {
        YAML::Node data = YAML::LoadFile(metadata.FilePath.string());
        if (!data["Material"]) {
            return nullptr;
        }
        
        auto materialNode = data["Material"];
        auto material = std::make_shared<Material>();
        
        if (auto n = materialNode["Type"]; n) {
            material->setMaterialType(static_cast<MaterialType>(n.as<int>()));
        }
        
        if (auto n = materialNode["Name"]; n) {
            material->setName(n.as<std::string>());
        }
        
        MaterialType type = material->getType();
        
        if (type == MaterialType::Phong) {
            if (auto n = materialNode["DiffuseColor"]; n && n.IsSequence() && n.size() == 4) {
                glm::vec4 diffuse(n[0].as<float>(), n[1].as<float>(), n[2].as<float>(), n[3].as<float>());
                material->setDiffuseColor(diffuse);
            }
            
            if (auto n = materialNode["AmbientColor"]; n && n.IsSequence() && n.size() == 4) {
                glm::vec4 ambient(n[0].as<float>(), n[1].as<float>(), n[2].as<float>(), n[3].as<float>());
                material->setAmbientColor(ambient);
            }
            
            if (auto n = materialNode["DiffuseTexture"]; n) {
                uint64_t textureHandle = n.as<uint64_t>();
                material->setDiffuseTexture(AssetHandle(textureHandle));
            }
        }
        else if (type == MaterialType::PBR) {
            if (auto n = materialNode["Albedo"]; n && n.IsSequence() && n.size() == 3) {
                glm::vec3 albedo(n[0].as<float>(), n[1].as<float>(), n[2].as<float>());
                material->setAlbedo(albedo);
            }
            
            if (auto n = materialNode["Metallic"]; n) {
                material->setMetallic(n.as<float>());
            }
            
            if (auto n = materialNode["Roughness"]; n) {
                material->setRoughness(n.as<float>());
            }
            
            if (auto n = materialNode["AO"]; n) {
                material->setAO(n.as<float>());
            }
            
            if (auto n = materialNode["AlbedoMap"]; n) {
                uint64_t handle = n.as<uint64_t>();
                material->setAlbedoMap(AssetHandle(handle));
            }
            
            if (auto n = materialNode["NormalMap"]; n) {
                uint64_t handle = n.as<uint64_t>();
                material->setNormalMap(AssetHandle(handle));
            }
            
            if (auto n = materialNode["MetallicMap"]; n) {
                uint64_t handle = n.as<uint64_t>();
                material->setMetallicMap(AssetHandle(handle));
            }
            
            if (auto n = materialNode["RoughnessMap"]; n) {
                uint64_t handle = n.as<uint64_t>();
                material->setRoughnessMap(AssetHandle(handle));
            }
            
            if (auto n = materialNode["AOMap"]; n) {
                uint64_t handle = n.as<uint64_t>();
                material->setAOMap(AssetHandle(handle));
            }
        }

        material->handle = metadata.Handle;
        
        return material;
        
    } catch (const std::exception &e) {
        return nullptr;
    }
}

} // namespace Fermion