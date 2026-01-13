#include "Asset/Loader/MeshLoader.hpp"
#include "Renderer/Model/MeshSerializer.hpp"

namespace Fermion
{
    std::shared_ptr<Asset> MeshLoader::load(const AssetMetadata &metadata)
    {
        // TODO:Mesh Deserialization
        auto mesh = MeshSerializer::deserialize(metadata.FilePath, metadata.Handle);
        FERMION_ASSERT(mesh != nullptr, "Failed to load mesh!");
        return mesh;
    }
} // namespace Fermion
