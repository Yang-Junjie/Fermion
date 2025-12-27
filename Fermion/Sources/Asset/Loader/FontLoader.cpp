#include "Asset/Loader/FontLoader.hpp"
#include "Renderer/Font/Font.hpp"

namespace Fermion {
std::shared_ptr<Asset> FontLoader::load(const AssetMetadata &metadata) {
    auto font = std::make_shared<Font>(metadata.FilePath);
    return font;
}
} // namespace Fermion
