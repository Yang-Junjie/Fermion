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

        if (options.includeEditorData)
        {
            const auto &editorData = material.getEditorData();
            out << YAML::Key << "Editor" << YAML::Value;
            out << YAML::BeginMap;
            out << YAML::Key << "NextNodeID" << YAML::Value << editorData.NextNodeID;
            out << YAML::Key << "NextLinkID" << YAML::Value << editorData.NextLinkID;

            out << YAML::Key << "Nodes" << YAML::Value;
            out << YAML::BeginSeq;
            for (const auto &node : editorData.Nodes)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "ID" << YAML::Value << node.ID;
                out << YAML::Key << "Type" << YAML::Value << node.Type;
                out << YAML::Key << "Position" << YAML::Value << YAML::Flow
                    << YAML::BeginSeq << node.PosX << node.PosY << YAML::EndSeq;
                if (node.Type == 1)
                {
                    out << YAML::Key << "TextureHandle" << YAML::Value
                        << static_cast<uint64_t>(node.TextureHandle);
                }
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;

            out << YAML::Key << "Links" << YAML::Value;
            out << YAML::BeginSeq;
            for (const auto &link : editorData.Links)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "ID" << YAML::Value << link.ID;
                out << YAML::Key << "StartPin" << YAML::Value << link.StartPinID;
                out << YAML::Key << "EndPin" << YAML::Value << link.EndPinID;
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;

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

            if (auto editorNode = data["Editor"]; editorNode)
            {
                MaterialNodeEditorData editorData;
                if (auto n = editorNode["NextNodeID"]; n)
                    editorData.NextNodeID = n.as<int>();
                if (auto n = editorNode["NextLinkID"]; n)
                    editorData.NextLinkID = n.as<int>();

                if (auto nodesNode = editorNode["Nodes"]; nodesNode && nodesNode.IsSequence())
                {
                    for (const auto &nodeNode : nodesNode)
                    {
                        MaterialNodeEditorData::NodeData nd;
                        if (nodeNode["ID"])
                            nd.ID = nodeNode["ID"].as<int>();
                        if (nodeNode["Type"])
                            nd.Type = nodeNode["Type"].as<int>();
                        if (auto pos = nodeNode["Position"]; pos && pos.IsSequence() && pos.size() == 2)
                        {
                            nd.PosX = pos[0].as<float>();
                            nd.PosY = pos[1].as<float>();
                        }
                        if (nd.Type == 1 && nodeNode["TextureHandle"])
                            nd.TextureHandle = AssetHandle(nodeNode["TextureHandle"].as<uint64_t>());
                        editorData.Nodes.push_back(nd);
                    }
                }

                if (auto linksNode = editorNode["Links"]; linksNode && linksNode.IsSequence())
                {
                    for (const auto &linkNode : linksNode)
                    {
                        MaterialNodeEditorData::LinkData ld;
                        if (linkNode["ID"])
                            ld.ID = linkNode["ID"].as<int>();
                        if (linkNode["StartPin"])
                            ld.StartPinID = linkNode["StartPin"].as<int>();
                        if (linkNode["EndPin"])
                            ld.EndPinID = linkNode["EndPin"].as<int>();
                        editorData.Links.push_back(ld);
                    }
                }

                material->setEditorData(editorData);
            }

            return material;
        }
        catch (...)
        {
            return nullptr;
        }
    }

} // namespace Fermion
