#pragma once
#include "Asset/Asset.hpp"
#include <filesystem>

namespace Fermion
{
    struct MSDFData;
    class Texture2D;
    class Font : public Asset
    {
    public:
        Font(const std::filesystem::path &font);

        ~Font();

        const MSDFData *getMSDFData() const
        {
            return m_data;
        }

        std::shared_ptr<Texture2D> getAtlasTexture() const;

        static std::shared_ptr<Font> getDefault();

        AssetType getAssetsType() const override
        {
            return AssetType::Font;
        }

    private:
        MSDFData *m_data;
        std::shared_ptr<Texture2D> m_atlasTexture;
    };
} // namespace Fermion
