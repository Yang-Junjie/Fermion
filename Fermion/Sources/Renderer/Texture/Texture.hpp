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
        RGBA32F,
        RGB16F,    // IBL: 辐照度贴图和预过滤贴图
        RG16F      // IBL: BRDF查找表
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
        static std::unique_ptr<Texture2D> create(uint32_t width, uint32_t height);
        static std::unique_ptr<Texture2D> create(const std::string &path);
        static std::unique_ptr<Texture2D> create(const TextureSpecification &spec);
        
        virtual void copyFromFramebuffer(std::shared_ptr<class Framebuffer> fb, uint32_t x, uint32_t y) = 0;
    };

    enum class TextureCubeFace : uint8_t
    {
        Front = 4,
        Back = 5,
        Left = 1,
        Right = 0,
        Up = 2,
        Down = 3
    };

    struct TextureCubeSpecification
    {
        // 运行时创建选项
        uint32_t width = 512;
        uint32_t height = 512;
        ImageFormat format = ImageFormat::RGB8;
        bool generateMips = false;
        uint32_t maxMipLevels = 1;
        
        // 从文件加载选项
        std::filesystem::path path;
        std::unordered_map<TextureCubeFace, std::string> names;
        bool flip = false;
    };

    class TextureCube : public Texture
    {
    public:
        virtual ~TextureCube() = default;
        static std::unique_ptr<TextureCube> create(const std::string &path);
        static std::unique_ptr<TextureCube> create(const TextureCubeSpecification &spec);
        
        virtual void copyFromFramebuffer(std::shared_ptr<class Framebuffer> fb, uint32_t face, uint32_t mipLevel) = 0;
    };
}