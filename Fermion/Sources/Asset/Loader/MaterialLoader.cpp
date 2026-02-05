#include "MaterialLoader.hpp"
#include "Renderer/Model/MaterialSerializer.hpp"

namespace Fermion {

std::shared_ptr<Asset> MaterialLoader::load(const AssetMetadata &metadata) {
    auto material = MaterialSerializer::deserialize(metadata.FilePath, metadata.Handle);
    FERMION_ASSERT(material != nullptr, "Failed to load material!");
    return material;
}

} // namespace Fermion
