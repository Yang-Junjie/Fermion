#include "Asset/Loader/ModelLoader.hpp"

#include "Renderer/Model/ModelSerializer.hpp"

namespace Fermion
{
    std::shared_ptr<Asset> ModelLoader::load(const AssetMetadata &metadata)
    {
        auto model = ModelSerializer::deserialize(metadata.FilePath, metadata.Handle);
        FERMION_ASSERT(model != nullptr, "Failed to load model!");
        return model;
    }
} // namespace Fermion
