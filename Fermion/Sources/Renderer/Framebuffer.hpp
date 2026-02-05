#pragma once
#include "fmpch.hpp"
namespace Fermion
{

    enum class FramebufferTextureFormat
    {
        None = 0,

        // Color
        RGBA8,
        RED_INTEGER,
        RGB16F,  // IBL: 辐照度贴图和预过滤贴图
        RGBA16F, // Advanced materials: GBuffer with alpha channel
        RG16F,   // IBL: BRDF查找表

        // Depth/stencil
        DEPTH24STENCIL8,
        DEPTH_COMPONENT32F, // For shadow mapping

        // Defaults
        Depth = DEPTH24STENCIL8
    };

    struct FramebufferTextureSpecification
    {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat format) : textureFormat(format)
        {
        }

        FramebufferTextureFormat textureFormat = FramebufferTextureFormat::None;
        // TODO: filtering/wrap
    };

    struct FramebufferAttachmentSpecification
    {
        FramebufferAttachmentSpecification() = default;
        FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments) : attachments(attachments)
        {
        }

        std::vector<FramebufferTextureSpecification> attachments;
    };

    struct FramebufferSpecification
    {
        uint32_t width = 0, height = 0;
        FramebufferAttachmentSpecification attachments;
        uint32_t samples = 1;

        bool swapChainTarget = false;
    };

    enum class FramebufferBlitMask : uint32_t
    {
        None = 0,
        Color = 1 << 0,
        Depth = 1 << 1,
        Stencil = 1 << 2
    };

    inline FramebufferBlitMask operator|(FramebufferBlitMask a, FramebufferBlitMask b)
    {
        return static_cast<FramebufferBlitMask>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline FramebufferBlitMask operator&(FramebufferBlitMask a, FramebufferBlitMask b)
    {
        return static_cast<FramebufferBlitMask>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    inline bool hasBlitMask(FramebufferBlitMask value, FramebufferBlitMask mask)
    {
        return (static_cast<uint32_t>(value) & static_cast<uint32_t>(mask)) != 0;
    }

    enum class FramebufferBlitFilter
    {
        Nearest,
        Linear
    };

    struct FramebufferBlitSpecification
    {
        uint32_t srcAttachmentIndex = 0;
        uint32_t dstAttachmentIndex = 0;
        FramebufferBlitMask mask = FramebufferBlitMask::Color;
        FramebufferBlitFilter filter = FramebufferBlitFilter::Nearest;
    };

    class Framebuffer
    {
    public:
        virtual ~Framebuffer() = default;

        virtual void bind() = 0;
        virtual void unbind() = 0;

        // Binds the framebuffer for reading (uses resolve FBO for MSAA)
        virtual void bindForRead() = 0;

        virtual void resize(uint32_t width, uint32_t height) = 0;
        virtual int readPixel(uint32_t attachmentIndex, int x, int y) = 0;

        virtual void clearAttachment(uint32_t attachmentIndex, int value) = 0;

        virtual uint32_t getColorAttachmentRendererID(uint32_t index = 0) const = 0;

        virtual uint32_t getDepthAttachmentRendererID() const = 0;

        virtual void bindColorAttachment(uint32_t attachmentIndex, uint32_t slot = 0) const = 0;

        virtual void bindDepthAttachment(uint32_t slot = 0) const = 0;

        virtual void blitTo(const std::shared_ptr<Framebuffer> &target, const FramebufferBlitSpecification &spec) const = 0;

        virtual void blitToDefaultFramebuffer(uint32_t dstWidth, uint32_t dstHeight, const FramebufferBlitSpecification &spec) const = 0;

        virtual void resolve() = 0;

        virtual bool isMultisampled() const { return getSpecification().samples > 1; }

        virtual const FramebufferSpecification &getSpecification() const = 0;

        static std::shared_ptr<Framebuffer> create(const FramebufferSpecification &spec);

        static void blit(const std::shared_ptr<Framebuffer> &source, const std::shared_ptr<Framebuffer> &target,
                         const FramebufferBlitSpecification &spec);

        static void blitToDefault(const std::shared_ptr<Framebuffer> &source,
                                  uint32_t dstWidth, uint32_t dstHeight,
                                  const FramebufferBlitSpecification &spec);
    };

} // namespace Fermion
