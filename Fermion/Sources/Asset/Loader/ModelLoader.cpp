#include "Asset/Loader/ModelLoader.hpp"

#include "Renderer/Model/ModelSerializer.hpp"

namespace Fermion
{
    std::shared_ptr<Asset> ModelLoader::load(const AssetMetadata &metadata)
    {
        return ModelSerializer::deserialize(metadata.FilePath, metadata.Handle);
    }
} // namespace Fermion
