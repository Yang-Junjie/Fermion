#include "Asset/Loader/SceneLoader.hpp"

#include "Scene/Scene.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Asset/SceneAsset.hpp"

namespace Fermion {
std::shared_ptr<Asset> SceneLoader::load(const AssetMetadata &metadata) {
    auto scene = std::make_shared<Scene>();
    SceneSerializer serializer(scene);

    if (!serializer.deserialize(metadata.FilePath))
        return nullptr;

    auto sceneAsset = std::make_shared<SceneAsset>(scene);
    return sceneAsset;
}
} // namespace Fermion
