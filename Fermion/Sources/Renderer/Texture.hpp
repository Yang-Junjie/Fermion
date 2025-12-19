#pragma once
#include "fmpch.hpp"
#include "Asset/Asset.hpp"
namespace Fermion
{
    enum class ImageFormat
    {
        None = 0,
        R8,
        RGB8,
        RGBA8,
        RGBA32F
    };

    struct TextureSpecification
    {
        uint32_t Width = 1;
        uint32_t Height = 1;
        ImageFormat Format = ImageFormat::RGBA8;
        bool GenerateMips = true;
    };
    class Texture : public Asset
    {
    public:
        virtual ~Texture() = default;
        virtual const TextureSpecification &getSpecification() const = 0;

        virtual void bind(uint32_t slot = 0) const = 0;

        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;
        virtual uint32_t getRendererID() const = 0;
        virtual const std::string &getPath() const = 0;

        virtual void setData(void *data, uint32_t size) = 0;
        virtual bool isLoaded() const = 0;

        virtual bool operator==(const Texture &other) const = 0;
        virtual AssetType getAssetsType() const override { return AssetType::Texture; }
    };
    class Texture2D : public Texture
    {
    public:
        static std::shared_ptr<Texture2D> create(uint32_t width, uint32_t height);
        static std::shared_ptr<Texture2D> create(const std::string &path);
        static std::shared_ptr<Texture2D> create(const TextureSpecification &spec);
    };

    class TextureCube : public Texture
    {
    public:
        static std::shared_ptr<TextureCube> create(const std::string &path);
    };
}