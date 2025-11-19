#include "AssetManager.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/Font.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Asset/SceneAsset.hpp"
#include "Project/Project.hpp"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Fermion
{
    std::unordered_map<AssetHandle, std::shared_ptr<Asset>> AssetManager::s_loadedAssets;
    std::filesystem::path AssetManager::s_assetDirectory;

    static AssetType GetAssetTypeFromPath(const std::filesystem::path &path)
    {
        auto ext = path.extension().string();
        auto it = s_AssetExtensionMap.find(ext);
        if (it != s_AssetExtensionMap.end())
            return it->second;
        return AssetType::None;
    }

    static std::filesystem::path GetMetaPath(const std::filesystem::path &assetPath)
    {
        std::filesystem::path metaPath = assetPath;
        metaPath += ".fmasset";
        return metaPath;
    }

    // Hazel-style: each asset has a sidecar .fmasset meta file which stores
    // a persistent UUID. That UUID is our AssetHandle and survives restarts.
    static AssetHandle LoadOrCreateAssetMeta(const std::filesystem::path &assetPath, AssetType type)
    {
        if (type == AssetType::None)
            return AssetHandle(0);

        std::filesystem::path metaPath = GetMetaPath(assetPath);

        if (std::filesystem::exists(metaPath))
        {
            try
            {
                YAML::Node data = YAML::LoadFile(metaPath.string());
                auto assetNode = data["Asset"];
                if (assetNode && assetNode["Handle"])
                {
                    uint64_t handleValue = assetNode["Handle"].as<uint64_t>();
                    return AssetHandle(handleValue);
                }
            }
            catch (const YAML::Exception &)
            {
                // fall through and recreate meta
            }
        }

        AssetHandle handle;
        if ((uint64_t)handle == 0)
            handle = AssetHandle(1);

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

        return handle;
    }

    void AssetManager::init(const std::filesystem::path &assetDirectory)
    {
        s_assetDirectory = assetDirectory;
        AssetRegistry::clear();

        if (!std::filesystem::exists(assetDirectory))
            return;

        for (auto &entry : std::filesystem::recursive_directory_iterator(assetDirectory))
        {
            if (!entry.is_regular_file())
                continue;

            const auto &path = entry.path();
            AssetType type = GetAssetTypeFromPath(path);
            if (type == AssetType::None)
                continue;

            AssetHandle handle = LoadOrCreateAssetMeta(path, type);
            if ((uint64_t)handle == 0)
                continue;

            AssetInfo info;
            info.Handle = handle;
            info.Type = type;
            info.FilePath = path;
            info.Name = path.stem().string();
            info.isMemoryAsset = false;

            AssetRegistry::set(info.Handle, info);
        }
    }

    void AssetManager::shutdown()
    {
        s_loadedAssets.clear();
        AssetRegistry::clear();
        s_assetDirectory.clear();
    }

    bool AssetManager::isAssetLoaded(AssetHandle handle)
    {
        return s_loadedAssets.find(handle) != s_loadedAssets.end();
    }

    void AssetManager::unloadAsset(AssetHandle handle)
    {
        s_loadedAssets.erase(handle);
    }

    void AssetManager::reloadAsset(AssetHandle handle)
    {
        unloadAsset(handle);
        if (AssetRegistry::exists(handle))
            loadAssetInternal(handle);
    }

    AssetHandle AssetManager::importAsset(const std::filesystem::path &path)
    {
        if (path.empty())
            return AssetHandle(0);

        std::filesystem::path absolutePath = std::filesystem::absolute(path);
        if (!std::filesystem::exists(absolutePath))
            return AssetHandle(0);

        AssetType type = GetAssetTypeFromPath(absolutePath);
        if (type == AssetType::None)
            return AssetHandle(0);

        AssetHandle handle = LoadOrCreateAssetMeta(absolutePath, type);
        if ((uint64_t)handle == 0)
            return AssetHandle(0);

        if (AssetRegistry::exists(handle))
            return handle;

        AssetInfo info;
        info.Handle = handle;
        info.Type = type;
        info.FilePath = absolutePath;
        info.Name = absolutePath.stem().string();
        info.isMemoryAsset = false;

        AssetRegistry::set(info.Handle, info);
        return info.Handle;
    }

    std::shared_ptr<Asset> AssetManager::loadAssetInternal(AssetHandle handle)
    {
        auto &info = AssetRegistry::get(handle);
        std::shared_ptr<Asset> asset;

        switch (info.Type)
        {
        case AssetType::Texture:
            asset = Texture2D::create(info.FilePath.string());
            break;
        case AssetType::Font:
            asset = std::make_shared<Font>(info.FilePath);
            break;
        case AssetType::Scene:
        {
            auto scene = std::make_shared<Scene>();
            SceneSerializer serializer(scene);
            serializer.deserialize(info.FilePath);
            asset = std::make_shared<SceneAsset>(scene);
            break;
        }
        default:
            break;
        }

        if (asset)
            asset->handle = handle;
        return asset;
    }
}

