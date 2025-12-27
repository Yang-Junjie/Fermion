#pragma once

#include "Renderer/Texture/Texture.hpp"
#include "AssetLoader.hpp"

namespace Fermion {
class TextureLoader : public AssetLoader {
public:
    std::shared_ptr<Asset> load(const AssetMetadata &metadata) override;
};
} // namespace Fermion
