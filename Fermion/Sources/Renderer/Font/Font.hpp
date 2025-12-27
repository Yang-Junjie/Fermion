#pragma once

#include <filesystem>

#include "../Texture/Texture.hpp"
#include "Asset/Asset.hpp"

namespace Fermion {

struct MSDFData;

class Font : public Asset {
public:
    Font(const std::filesystem::path &font);
    ~Font();

    const MSDFData *getMSDFData() const {
        return m_data;
    }
    std::shared_ptr<Texture2D> getAtlasTexture() const {
        return m_atlasTexture;
    }

    static std::shared_ptr<Font> getDefault();
    virtual AssetType getAssetsType() const override {
        return AssetType::Font;
    }

private:
    MSDFData *m_data;
    std::shared_ptr<Texture2D> m_atlasTexture;
};

} // namespace Fermion
