#pragma once

#include "Renderer/Framebuffer.hpp"

namespace Fermion {

class OpenGLFramebuffer : public Framebuffer {
public:
    OpenGLFramebuffer(const FramebufferSpecification &spec);
    virtual ~OpenGLFramebuffer();

    void invalidate();

    virtual void bind() override;
    virtual void unbind() override;

    // Binds the resolve FBO for MSAA, or main FBO if not multisampled
    virtual void bindForRead() override;

    virtual void resize(uint32_t width, uint32_t height) override;
    virtual int readPixel(uint32_t attachmentIndex, int x, int y) override;

    virtual void clearAttachment(uint32_t attachmentIndex, int value) override;

    virtual uint32_t getColorAttachmentRendererID(uint32_t index = 0) const override {
        // For MSAA, return the resolved texture (for shader sampling)
        if (isMultisampled() && index < m_resolveColorAttachments.size()) {
            return m_resolveColorAttachments[index];
        }
        return m_colorAttachments[index];
    }

    virtual uint32_t getDepthAttachmentRendererID() const override {
        // For MSAA, return the resolved depth texture
        if (isMultisampled() && m_resolveDepthAttachment) {
            return m_resolveDepthAttachment;
        }
        return m_depthAttachment;
    }

    // Returns the raw MSAA texture (for advanced use cases)
    uint32_t getMSAAColorAttachmentRendererID(uint32_t index = 0) const {
        return m_colorAttachments[index];
    }

    uint32_t getMSAADepthAttachmentRendererID() const {
        return m_depthAttachment;
    }
    
    virtual void bindColorAttachment(uint32_t attachmentIndex, uint32_t slot = 0) const override;

    virtual void bindDepthAttachment(uint32_t slot = 0) const override;

    virtual void blitTo(const std::shared_ptr<Framebuffer> &target, const FramebufferBlitSpecification &spec) const override;

    virtual void resolve() override;

    virtual const FramebufferSpecification &getSpecification() const override {
        return m_specification;
    }

private:
    uint32_t m_rendererID = 0;
    FramebufferSpecification m_specification;

    std::vector<FramebufferTextureSpecification> m_colorAttachmentSpecifications;
    FramebufferTextureSpecification m_depthAttachmentSpecification = FramebufferTextureFormat::None;

    std::vector<uint32_t> m_colorAttachments;
    uint32_t m_depthAttachment = 0;

    // Resolve framebuffer (used when samples > 1)
    uint32_t m_resolveRendererID = 0;
    std::vector<uint32_t> m_resolveColorAttachments;
    uint32_t m_resolveDepthAttachment = 0;
};

} // namespace Fermion
