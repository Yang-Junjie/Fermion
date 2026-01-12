#include "MaterialLoader.hpp"
#include "Renderer/Model/MaterialSerializer.hpp"

namespace Fermion {

std::shared_ptr<Asset> MaterialLoader::load(const AssetMetadata &metadata) {
    return MaterialSerializer::deserialize(metadata.FilePath, metadata.Handle);
}

} // namespace Fermion
