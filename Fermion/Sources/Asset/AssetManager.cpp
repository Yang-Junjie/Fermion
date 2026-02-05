#include "AssetManager.hpp"
#include "Asset/Asset.hpp"
#include "Asset/AssetSerializer.hpp"


#include "Asset/Importer/TextureImporter.hpp"
#include "Asset/Importer/FontImporter.hpp"
#include "Asset/Importer/SceneImporter.hpp"
#include "Asset/Importer/ShaderImporter.hpp"
#include "Asset/Importer/MeshImporter.hpp"
#include "Asset/Importer/MaterialImporter.hpp"
#include "Asset/Importer/ModelSourceImporter.hpp"
#include "Asset/Importer/ModelImporter.hpp"

#include "Asset/Loader/TextureLoader.hpp"
#include "Asset/Loader/FontLoader.hpp"
#include "Asset/Loader/MeshLoader.hpp"
#include "Asset/Loader/SceneLoader.hpp"
#include "Asset/Loader/MaterialLoader.hpp"
#include "Asset/Loader/ModelLoader.hpp"
#include "Asset/Loader/SkeletonLoader.hpp"
#include "Asset/Loader/AnimationClipLoader.hpp"
#include "Asset/AssetExtensions.hpp"

namespace Fermion
{
    std::unordered_map<AssetHandle, std::shared_ptr<Asset>> AssetManager::s_loadedAssets;
    std::filesystem::path AssetManager::s_assetDirectory;
    std::unordered_map<AssetType, std::unique_ptr<AssetLoader>, AssetManager::AssetTypeHash> AssetManager::s_assetLoaders;
    std::unordered_map<AssetType, std::shared_ptr<Asset>> AssetManager::s_defaultAssets;
    void AssetManager::ensureDefaultLoaders()
    {
        if (!s_assetLoaders.empty())
            return;

        s_assetLoaders.emplace(AssetType::Texture, std::make_unique<TextureLoader>());
        s_assetLoaders.emplace(AssetType::Font, std::make_unique<FontLoader>());
        s_assetLoaders.emplace(AssetType::Scene, std::make_unique<SceneLoader>());
        s_assetLoaders.emplace(AssetType::Mesh, std::make_unique<MeshLoader>());
        s_assetLoaders.emplace(AssetType::Material, std::make_unique<MaterialLoader>());
        s_assetLoaders.emplace(AssetType::Model, std::make_unique<ModelLoader>());
        s_assetLoaders.emplace(AssetType::Skeleton, std::make_unique<SkeletonLoader>());
        s_assetLoaders.emplace(AssetType::AnimationClip, std::make_unique<AnimationClipLoader>());
    }

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
        metaPath += ".meta";
        return metaPath;
    }

    void AssetManager::init(const std::filesystem::path &assetDirectory)
    {
        s_assetDirectory = assetDirectory;

        s_assetLoaders.clear();
        ensureDefaultLoaders();

        for (auto &p : std::filesystem::recursive_directory_iterator(assetDirectory))
        {
            if (!p.is_regular_file())
                continue;

            AssetType type = GetAssetTypeFromPath(p.path());
            if (type == AssetType::None)
                continue;

            if (type == AssetType::TextureSource)
            {
                auto assetAbsolutePath = std::filesystem::absolute(p.path());
                auto expectedFtexPath = p.path();
                expectedFtexPath.replace_extension(".ftex");

                if (!std::filesystem::exists(expectedFtexPath))
                {
                    TextureImporter::generateDefaultFtex(assetAbsolutePath);
                }
                continue;
            }

            if (type == AssetType::ModelSource)
            {
                auto metaPath = GetMetaPath(p.path());
                auto assetAbsolutePath = std::filesystem::absolute(p.path());

                auto expectedModelPath = p.path().parent_path() / p.path().stem();
                expectedModelPath.replace_extension(".fmodel");

                if (!std::filesystem::exists(expectedModelPath))
                {

                    auto importer = std::make_unique<ModelSourceImporter>();
                    AssetMetadata modelMeta = importer->importAsset(assetAbsolutePath);

                    if (modelMeta.Type == AssetType::Model)
                    {
                        importer->writeMetadata(modelMeta);
                        AssetRegistry::set(modelMeta.Handle, modelMeta);
                    }
                }
                continue;
            }

            auto metaPath = GetMetaPath(p.path());
            auto assetAbsolutePath = std::filesystem::absolute(p.path());

            AssetMetadata meta = AssetSerializer::deserializeMeta(metaPath);
            bool shouldWriteMeta = false;

            if (meta.Type == AssetType::None)
            {
                meta.Handle = AssetHandle();
                meta.Type = type;
                meta.Name = p.path().stem().string();
                shouldWriteMeta = true;
            }

            if (meta.FilePath != assetAbsolutePath)
            {
                meta.FilePath = assetAbsolutePath;
                shouldWriteMeta = true;
            }

            if (shouldWriteMeta)
                AssetSerializer::serializeMeta(metaPath, meta);

            AssetRegistry::set(meta.Handle, meta);
        }

        uint32_t p = 0xFFF527D6;
        uint32_t w = 0xFFFFFFFF;

        std::vector<uint32_t> data = {
            w, p,
            p, w};

        std::shared_ptr<Texture2D> defaultTexture = Texture2D::create(2, 2);
        defaultTexture->setData(data.data(), sizeof(uint32_t) * data.size());
        s_defaultAssets[AssetType::Texture] = defaultTexture;
    }

    void AssetManager::shutdown()
    {
        s_loadedAssets.clear();
        AssetRegistry::clear();
        s_assetLoaders.clear();
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

    static std::unique_ptr<AssetImporter> CreateImporter(AssetType type)
    {
        switch (type)
        {
        case AssetType::Texture:
            return std::make_unique<TextureImporter>();
        case AssetType::Font:
            return std::make_unique<FontImporter>();
        case AssetType::Scene:
            return std::make_unique<SceneImporter>();
        case AssetType::Shader:
            return std::make_unique<ShaderImporter>();
        case AssetType::Mesh:
            return std::make_unique<MeshImporter>();
        case AssetType::Material:
            return std::make_unique<MaterialImporter>();
        case AssetType::ModelSource:
            return std::make_unique<ModelSourceImporter>();
        case AssetType::Model:
            return std::make_unique<ModelImporter>();
        default:
            return nullptr;
        }
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

        if (type == AssetType::TextureSource)
        {
            auto ftexPath = absolutePath;
            ftexPath.replace_extension(".ftex");
            if (!std::filesystem::exists(ftexPath))
            {
                TextureImporter::generateDefaultFtex(absolutePath);
            }
            return importAsset(ftexPath);
        }

        auto metaPath = GetMetaPath(absolutePath);

        AssetMetadata metadata = AssetSerializer::deserializeMeta(metaPath);
        if (metadata.Type == type && static_cast<uint64_t>(metadata.Handle) != 0)
        {
            bool shouldWriteMeta = false;
            if (metadata.FilePath != absolutePath)
            {
                metadata.FilePath = absolutePath;
                shouldWriteMeta = true;
            }

            const auto expectedName = absolutePath.stem().string();
            if (metadata.Name != expectedName)
            {
                metadata.Name = expectedName;
                shouldWriteMeta = true;
            }

            if (shouldWriteMeta)
                AssetSerializer::serializeMeta(metaPath, metadata);

            if (!AssetRegistry::exists(metadata.Handle))
                AssetRegistry::set(metadata.Handle, metadata);

            return metadata.Handle;
        }

        auto importer = CreateImporter(type);
        if (importer)
        {
            if (metadata.Type != type)
            {
                metadata = importer->importAsset(absolutePath);
                importer->writeMetadata(metadata);
            }
            else
            {
                metadata.FilePath = absolutePath;
                metadata.Name = absolutePath.stem().string();
                importer->writeMetadata(metadata);
            }
        }
        else
        {
            if (metadata.Type == AssetType::None)
            {
                metadata.Handle = UUID();
                if (static_cast<uint64_t>(metadata.Handle) == 0)
                    metadata.Handle = UUID(1);

                metadata.Type = type;
                metadata.FilePath = absolutePath;
                metadata.Name = absolutePath.stem().string();

                AssetSerializer::serializeMeta(metaPath, metadata);
            }
            else
            {
                metadata.FilePath = absolutePath;
                metadata.Name = absolutePath.stem().string();
                AssetSerializer::serializeMeta(metaPath, metadata);
            }
        }

        if (!AssetRegistry::exists(metadata.Handle))
            AssetRegistry::set(metadata.Handle, metadata);

        return metadata.Handle;
    }

    AssetHandle AssetManager::addMemoryOnlyAsset(std::shared_ptr<Asset> asset)
    {
        asset->handle = AssetHandle();
        s_loadedAssets[asset->handle] = asset;
        return asset->handle;
    }

    void AssetManager::registerLoadedAsset(AssetHandle handle, std::shared_ptr<Asset> asset)
    {
        s_loadedAssets[handle] = asset;
    }

    std::shared_ptr<Asset> AssetManager::getAssetMetadata(AssetHandle handle)
    {
        return std::shared_ptr<Asset>();
    }

    std::shared_ptr<Asset> AssetManager::getDefaultAsset(AssetType type)
    {
        return s_defaultAssets[type];
    }

    std::shared_ptr<Asset> AssetManager::loadAssetInternal(AssetHandle handle)
    {
        auto &metadata = AssetRegistry::get(handle);
        std::shared_ptr<Asset> asset;

        ensureDefaultLoaders();

        auto loaderIt = s_assetLoaders.find(metadata.Type);
        if (loaderIt == s_assetLoaders.end() || !loaderIt->second)
            return nullptr;

        asset = loaderIt->second->load(metadata);

        if (asset)
            asset->handle = handle;

        return asset;
    }
} // namespace Fermion
