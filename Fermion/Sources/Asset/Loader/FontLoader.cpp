#include "Asset/Loader/FontLoader.hpp"
#include "Renderer/Font/Font.hpp"

namespace Fermion {
std::shared_ptr<Asset> FontLoader::load(const AssetMetadata &metadata) {
    auto font = std::make_shared<Font>(metadata.FilePath);
    FERMION_ASSERT(font != nullptr, "Failed to load font!");
    return font;
}
} // namespace Fermion
