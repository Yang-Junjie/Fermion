#include "Asset/Loader/TextureLoader.hpp"

namespace Fermion
{
    std::shared_ptr<Asset> TextureLoader::load(const AssetMetadata &metadata)
    {
        auto texture = Texture2D::create(metadata.FilePath.string());
        return texture;
    }

} // namespace Fermion
