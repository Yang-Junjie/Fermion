#pragma once
#include "IThumbnailProvider.hpp"
#include "Renderer/Preview/MaterialPreviewRenderer.hpp"
#include <memory>

namespace Fermion
{

    class MaterialThumbnailProvider : public IThumbnailProvider
    {
    public:
        MaterialThumbnailProvider();

        std::unique_ptr<Texture2D> generateThumbnail(
            const std::filesystem::path &path) override;

        bool supports(AssetType type) const override
        {
            return type == AssetType::Material;
        }

    private:
        std::unique_ptr<MaterialPreviewRenderer> m_renderer;
    };

} // namespace Fermion
