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
        static bool deserializeMeta(const std::filesystem::path& metaPath, AssetHandle& outHandle, AssetType& outType)
        {
            if (!std::filesystem::exists(metaPath)) return false;

            try
            {
                YAML::Node data = YAML::LoadFile(metaPath.string());
                auto assetNode = data["Asset"];
                if (!assetNode || !assetNode["Handle"] || !assetNode["Type"])
                    return false;

                uint64_t handleValue = assetNode["Handle"].as<uint64_t>();
                int typeValue = assetNode["Type"].as<int>();
                outHandle = AssetHandle(handleValue);
                outType = static_cast<AssetType>(typeValue);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        static void serializeMeta(const std::filesystem::path& metaPath, AssetHandle handle, AssetType type)
        {
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "Asset" << YAML::Value;
            out << YAML::BeginMap;
            out << YAML::Key << "Handle" << YAML::Value << static_cast<uint64_t>(handle);
            out << YAML::Key << "Type" << YAML::Value << static_cast<int>(type);
            out << YAML::EndMap;
            out << YAML::EndMap;

            std::ofstream fout(metaPath);
            fout << out.c_str();
        }
    };
}
