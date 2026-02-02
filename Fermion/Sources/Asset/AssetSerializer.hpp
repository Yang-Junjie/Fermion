#pragma once
#include "AssetMetadata.hpp"
#include "AssetTypes.hpp"
#include "Project/Project.hpp"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>
#include <system_error>

namespace Fermion
{
    namespace detail
    {
        inline std::filesystem::path GetProjectDirectorySafe()
        {
            if (Project::getActive())
                return Project::getProjectDirectory();
            return {};
        }

        inline std::string PathRelativeToProject(const std::filesystem::path &absolutePath)
        {
            if (absolutePath.empty())
                return {};

            auto normalized = absolutePath.lexically_normal();
            auto projectDir = GetProjectDirectorySafe();
            if (!projectDir.empty())
            {
                std::error_code ec;
                auto relative = std::filesystem::relative(normalized, projectDir, ec);
                if (!ec)
                    return relative.generic_string();
            }

            return normalized.generic_string();
        }

        inline std::filesystem::path ResolveStoredPath(const std::string &stored,
                                                       const std::filesystem::path &metaPath)
        {
            std::filesystem::path defaultPath = metaPath;
            defaultPath.replace_extension("");

            if (stored.empty())
                return defaultPath.lexically_normal();

            std::filesystem::path storedPath = std::filesystem::path(stored).lexically_normal();
            if (storedPath.is_absolute())
                return storedPath;

            auto projectDir = GetProjectDirectorySafe();
            if (!projectDir.empty())
                return (projectDir / storedPath).lexically_normal();

            return (metaPath.parent_path() / storedPath).lexically_normal();
        }
    } // namespace detail

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
                out << YAML::Key << "FilePath"
                    << YAML::Value << detail::PathRelativeToProject(metadata.FilePath);
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
                std::string storedPath;
                if (assetNode["FilePath"])
                    storedPath = assetNode["FilePath"].as<std::string>();

                metadata.Handle = handle;
                metadata.Type = AssetUtils::StringToAssetType(type);
                metadata.Name = name;
                metadata.FilePath = detail::ResolveStoredPath(storedPath, metaPath);

                if (storedPath != detail::PathRelativeToProject(metadata.FilePath))
                    serializeMeta(metaPath, metadata);

                return metadata;
            }
            catch (...)
            {
                return {};
            }
        }
    };
} // namespace Fermion
