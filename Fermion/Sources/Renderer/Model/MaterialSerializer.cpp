#include "MaterialSerializer.hpp"

#include <yaml-cpp/yaml.h>

#include <fstream>
#include "Core/Yaml.hpp"
namespace Fermion
{

    bool MaterialSerializer::serialize(const std::filesystem::path &filepath, const Material &material,
                                       const MaterialSerializeOptions &options)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Material" << YAML::Value;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Type" << YAML::Value << static_cast<int>(material.getType());
            out << YAML::Key << "Name" << YAML::Value << material.getName();

            const MaterialType type = material.getType();
            if (type == MaterialType::Phong)
            {
                out << YAML::Key << "DiffuseColor" << YAML::Value << material.getDiffuseColor();
                out << YAML::Key << "AmbientColor" << YAML::Value << material.getAmbientColor();
                out << YAML::Key << "DiffuseTexture" << YAML::Value
                    << static_cast<uint64_t>(material.getDiffuseTexture());
            }
            else if (type == MaterialType::PBR)
            {
                out << YAML::Key << "Albedo" << YAML::Value << material.getAlbedo();
                out << YAML::Key << "Metallic" << YAML::Value << material.getMetallic();
                out << YAML::Key << "Roughness" << YAML::Value << material.getRoughness();
                out << YAML::Key << "AO" << YAML::Value << material.getAO();

                if (options.includePBRMaps)
                {
                    out << YAML::Key << "AlbedoMap" << YAML::Value
                        << static_cast<uint64_t>(material.getAlbedoMap());
                    out << YAML::Key << "NormalMap" << YAML::Value
                        << static_cast<uint64_t>(material.getNormalMap());
                    out << YAML::Key << "MetallicMap" << YAML::Value
                        << static_cast<uint64_t>(material.getMetallicMap());
                    out << YAML::Key << "RoughnessMap" << YAML::Value
                        << static_cast<uint64_t>(material.getRoughnessMap());
                    out << YAML::Key << "AOMap" << YAML::Value
                        << static_cast<uint64_t>(material.getAOMap());
                }
            }

            out << YAML::EndMap;
        }
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        if (!fout.is_open())
            return false;

        fout << out.c_str();
        return true;
    }

    std::shared_ptr<Material> MaterialSerializer::deserialize(const std::filesystem::path &filepath,
                                                              AssetHandle handle)
    {
        if (!std::filesystem::exists(filepath))
            return nullptr;

        try
        {
            YAML::Node data = YAML::LoadFile(filepath.string());
            if (!data["Material"])
                return nullptr;

            auto materialNode = data["Material"];
            auto material = std::make_shared<Material>();

            if (auto n = materialNode["Type"]; n)
                material->setMaterialType(static_cast<MaterialType>(n.as<int>()));
            if (auto n = materialNode["Name"]; n)
                material->setName(n.as<std::string>());

            const MaterialType type = material->getType();
            if (type == MaterialType::Phong)
            {
                if (auto n = materialNode["DiffuseColor"]; n && n.IsSequence() && n.size() == 4)
                {
                    material->setDiffuseColor(glm::vec4(n[0].as<float>(), n[1].as<float>(),
                                                        n[2].as<float>(), n[3].as<float>()));
                }
                if (auto n = materialNode["AmbientColor"]; n && n.IsSequence() && n.size() == 4)
                {
                    material->setAmbientColor(glm::vec4(n[0].as<float>(), n[1].as<float>(),
                                                        n[2].as<float>(), n[3].as<float>()));
                }
                if (auto n = materialNode["DiffuseTexture"]; n)
                    material->setDiffuseTexture(AssetHandle(n.as<uint64_t>()));
            }
            else if (type == MaterialType::PBR)
            {
                if (auto n = materialNode["Albedo"]; n && n.IsSequence() && n.size() == 3)
                {
                    material->setAlbedo(
                        glm::vec3(n[0].as<float>(), n[1].as<float>(), n[2].as<float>()));
                }
                if (auto n = materialNode["Metallic"]; n)
                    material->setMetallic(n.as<float>());
                if (auto n = materialNode["Roughness"]; n)
                    material->setRoughness(n.as<float>());
                if (auto n = materialNode["AO"]; n)
                    material->setAO(n.as<float>());

                if (auto n = materialNode["AlbedoMap"]; n)
                    material->setAlbedoMap(AssetHandle(n.as<uint64_t>()));
                if (auto n = materialNode["NormalMap"]; n)
                    material->setNormalMap(AssetHandle(n.as<uint64_t>()));
                if (auto n = materialNode["MetallicMap"]; n)
                    material->setMetallicMap(AssetHandle(n.as<uint64_t>()));
                if (auto n = materialNode["RoughnessMap"]; n)
                    material->setRoughnessMap(AssetHandle(n.as<uint64_t>()));
                if (auto n = materialNode["AOMap"]; n)
                    material->setAOMap(AssetHandle(n.as<uint64_t>()));
            }

            material->handle = handle;
            return material;
        }
        catch (...)
        {
            return nullptr;
        }
    }

} // namespace Fermion
