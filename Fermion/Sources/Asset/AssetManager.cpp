#include "AssetManager.hpp"
#include "Renderer/Texture/Texture.hpp"
#include "Renderer/Font/Font.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Asset/SceneAsset.hpp"
#include "Asset/AssetSerializer.hpp"
#include "Asset/Importer/TextureImporter.hpp"
#include "Asset/Importer/FontImporter.hpp"
#include "Asset/Importer/SceneImporter.hpp"
#include "Asset/Importer/ShaderImporter.hpp"
#include "Asset/Importer/MeshImporter.hpp"
#include "Asset/Loader/TextureLoader.hpp"
#include "Asset/Loader/FontLoader.hpp"
#include "Asset/Loader/MeshLoader.hpp"
#include "Asset/Loader/SceneLoader.hpp"

namespace Fermion {
std::unordered_map<AssetHandle, std::shared_ptr<Asset>> AssetManager::s_loadedAssets;
std::filesystem::path AssetManager::s_assetDirectory;
std::unordered_map<AssetType, std::unique_ptr<AssetLoader>, AssetManager::AssetTypeHash> AssetManager::s_assetLoaders;

void AssetManager::ensureDefaultLoaders() {
    if (!s_assetLoaders.empty())
        return;

    s_assetLoaders.emplace(AssetType::Texture, std::make_unique<TextureLoader>());
    s_assetLoaders.emplace(AssetType::Font, std::make_unique<FontLoader>());
    s_assetLoaders.emplace(AssetType::Scene, std::make_unique<SceneLoader>());
    s_assetLoaders.emplace(AssetType::Mesh, std::make_unique<MeshLoader>());
}

static AssetType GetAssetTypeFromPath(const std::filesystem::path &path) {
    auto ext = path.extension().string();
    auto it = s_AssetExtensionMap.find(ext);
    if (it != s_AssetExtensionMap.end())
        return it->second;
    return AssetType::None;
}

static std::filesystem::path GetMetaPath(const std::filesystem::path &assetPath) {
    std::filesystem::path metaPath = assetPath;
    metaPath += ".meta";
    return metaPath;
}

void AssetManager::init(const std::filesystem::path &assetDirectory) {
    s_assetDirectory = assetDirectory;

    s_assetLoaders.clear();
    ensureDefaultLoaders();

    for (auto &p : std::filesystem::recursive_directory_iterator(assetDirectory)) {
        if (!p.is_regular_file())
            continue;

        AssetType type = GetAssetTypeFromPath(p.path());
        if (type == AssetType::None)
            continue;

        auto metaPath = GetMetaPath(p.path());

        AssetMetadata meta = AssetSerializer::deserializeMeta(metaPath);

        if (meta.Type == AssetType::None) {
            meta.Handle = AssetHandle();
            meta.Type = type;
            meta.Name = p.path().stem().string();
            meta.FilePath = std::filesystem::absolute(p.path());

            AssetSerializer::serializeMeta(metaPath, meta);
        } else {
            meta.FilePath = std::filesystem::absolute(p.path());
        }

        AssetRegistry::set(meta.Handle, meta);
    }
}

void AssetManager::shutdown() {
    s_loadedAssets.clear();
    AssetRegistry::clear();
    s_assetLoaders.clear();
    s_assetDirectory.clear();
}

bool AssetManager::isAssetLoaded(AssetHandle handle) {
    return s_loadedAssets.find(handle) != s_loadedAssets.end();
}

void AssetManager::unloadAsset(AssetHandle handle) {
    s_loadedAssets.erase(handle);
}

void AssetManager::reloadAsset(AssetHandle handle) {
    unloadAsset(handle);
    if (AssetRegistry::exists(handle))
        loadAssetInternal(handle);
}

static std::unique_ptr<AssetImporter> CreateImporter(AssetType type) {
    switch (type) {
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
    default:
        return nullptr;
    }
}

AssetHandle AssetManager::importAsset(const std::filesystem::path &path) {
    if (path.empty())
        return AssetHandle(0);

    std::filesystem::path absolutePath = std::filesystem::absolute(path);
    if (!std::filesystem::exists(absolutePath))
        return AssetHandle(0);

    AssetType type = GetAssetTypeFromPath(absolutePath);
    if (type == AssetType::None)
        return AssetHandle(0);

    auto metaPath = GetMetaPath(absolutePath);

    AssetMetadata metadata = AssetSerializer::deserializeMeta(metaPath);

    auto importer = CreateImporter(type);
    if (importer) {
        if (metadata.Type != type) {
            metadata = importer->importAsset(absolutePath);
            importer->writeMetadata(metadata);
        } else {
            metadata.FilePath = absolutePath;
            metadata.Name = absolutePath.stem().string();
            importer->writeMetadata(metadata);
        }
    } else {
        if (metadata.Type == AssetType::None) {
            metadata.Handle = UUID();
            if (static_cast<uint64_t>(metadata.Handle) == 0)
                metadata.Handle = UUID(1);

            metadata.Type = type;
            metadata.FilePath = absolutePath;
            metadata.Name = absolutePath.stem().string();

            AssetSerializer::serializeMeta(metaPath, metadata);
        } else {
            metadata.FilePath = absolutePath;
            metadata.Name = absolutePath.stem().string();
            AssetSerializer::serializeMeta(metaPath, metadata);
        }
    }

    if (!AssetRegistry::exists(metadata.Handle))
        AssetRegistry::set(metadata.Handle, metadata);

    return metadata.Handle;
}

AssetHandle AssetManager::addMemoryOnlyAsset(std::shared_ptr<Asset> asset) {
    return AssetHandle();
}

std::shared_ptr<Asset> AssetManager::getAssetMetadata(AssetHandle handle) {
    return std::shared_ptr<Asset>();
}

std::shared_ptr<Asset> AssetManager::loadAssetInternal(AssetHandle handle) {
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
