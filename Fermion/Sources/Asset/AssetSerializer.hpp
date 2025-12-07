#pragma once
#include "AssetMetadata.hpp"
#include "AssetTypes.hpp"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>

namespace Fermion
{
    class AssetSerializer
    {
    public:
        static void serializeMeta(const std::filesystem::path &metaPath, const AssetMetadata &metadata)
        {
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "Asset" << YAML::Value;
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Handle" << YAML::Value << static_cast<uint64_t>(metadata.Handle);
                out << YAML::Key << "Type" << YAML::Value << AssetUtils::AssetTypeToString(metadata.Type);
                out << YAML::Key << "Name" << YAML::Value << metadata.Name;
                out << YAML::Key << "FilePath" << YAML::Value << metadata.FilePath.string();
                out << YAML::EndMap;
            }
            out << YAML::EndMap;

            std::ofstream fout(metaPath);
            fout << out.c_str();
        }

        static AssetMetadata deserializeMeta(const std::filesystem::path &metaPath)
        {
            if (!std::filesystem::exists(metaPath))
                return {};

            try
            {
                AssetMetadata metadata;
                YAML::Node data = YAML::LoadFile(metaPath.string());
                auto assetNode = data["Asset"];
                if (!assetNode || !assetNode["Handle"] || !assetNode["Type"] || !assetNode["Name"])
                    return {};

                uint64_t handle = assetNode["Handle"].as<uint64_t>();
                std::string type = assetNode["Type"].as<std::string>();
                std::string name = assetNode["Name"].as<std::string>();

                metadata.Handle = handle;
                metadata.Type = AssetUtils::StringToAssetType(type);
                metadata.Name = name;
                metadata.FilePath = metaPath;
                metadata.FilePath.replace_extension("");

                return metadata;
            }
            catch (...)
            {
                return {};
            }
        }
    };
}
