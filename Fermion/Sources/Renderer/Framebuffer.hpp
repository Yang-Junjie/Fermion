#pragma once
#include "fmpch.hpp"
namespace Fermion {

enum class FramebufferTextureFormat {
    None = 0,

    // Color
    RGBA8,
    RED_INTEGER,

    // Depth/stencil
    DEPTH24STENCIL8,
    DEPTH_COMPONENT32F,  // For shadow mapping

    // Defaults
    Depth = DEPTH24STENCIL8
};

struct FramebufferTextureSpecification {
    FramebufferTextureSpecification() = default;
    FramebufferTextureSpecification(FramebufferTextureFormat format) : textureFormat(format) {
    }

    FramebufferTextureFormat textureFormat = FramebufferTextureFormat::None;
    // TODO: filtering/wrap
};

struct FramebufferAttachmentSpecification {
    FramebufferAttachmentSpecification() = default;
    FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments) : attachments(attachments) {
    }

    std::vector<FramebufferTextureSpecification> attachments;
};

struct FramebufferSpecification {
    uint32_t width = 0, height = 0;
    FramebufferAttachmentSpecification attachments;
    uint32_t samples = 1;

    bool swapChainTarget = false;
};

class Framebuffer {
public:
    virtual ~Framebuffer() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    virtual void resize(uint32_t width, uint32_t height) = 0;
    virtual int readPixel(uint32_t attachmentIndex, int x, int y) = 0;

    virtual void clearAttachment(uint32_t attachmentIndex, int value) = 0;

    virtual uint32_t getColorAttachmentRendererID(uint32_t index = 0) const = 0;
    
    virtual uint32_t getDepthAttachmentRendererID() const = 0;
    
    virtual void bindDepthAttachment(uint32_t slot = 0) const = 0;

    virtual const FramebufferSpecification &getSpecification() const = 0;

    static std::shared_ptr<Framebuffer> create(const FramebufferSpecification &spec);
};

} // namespace Fermion
