#pragma once
#include "Renderer/Texture/Texture.hpp"
#include "Asset/AssetTypes.hpp"
#include <filesystem>
#include <memory>

namespace Fermion
{

    class IThumbnailProvider
    {
    public:
        virtual ~IThumbnailProvider() = default;

        virtual std::unique_ptr<Texture2D> generateThumbnail(
            const std::filesystem::path &path) = 0;

        virtual bool supports(AssetType type) const = 0;
    };

} // namespace Fermion
