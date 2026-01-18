#include "fmpch.hpp"
#include "OpenGLFramebuffer.hpp"

#include <glad/glad.h>

namespace Fermion {

static const uint32_t s_MaxFramebufferSize = 8192;

namespace Utils {

static GLenum textureTarget(bool multisampled) {
    return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
}

static void createTextures(bool multisampled, uint32_t *outID, uint32_t count) {
    glCreateTextures(textureTarget(multisampled), count, outID);
}

static void bindTexture(bool multisampled, uint32_t id) {
    glBindTexture(textureTarget(multisampled), id);
}

static void attachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index) {
    bool multisampled = samples > 1;
    if (multisampled) {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
    } else {
        // Choose data type based on format
        GLenum type = GL_UNSIGNED_BYTE;
        if (format == GL_RED_INTEGER) {
            type = GL_INT;
        } else if (internalFormat == GL_RGB16F || internalFormat == GL_RG16F) {
            type = GL_FLOAT;
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);

        // Integer textures must use NEAREST filtering
        GLenum filter = (format == GL_RED_INTEGER) ? GL_NEAREST : GL_LINEAR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, textureTarget(multisampled), id, 0);
}

static void attachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height, bool isShadowMap = false) {
    bool multisampled = samples > 1;
    if (multisampled) {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
    } else {
        glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

        if (isShadowMap) {
            // Shadow map specific settings
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, textureTarget(multisampled), id, 0);
}

static bool isDepthFormat(FramebufferTextureFormat format) {
    switch (format) {
    case FramebufferTextureFormat::DEPTH24STENCIL8:
    case FramebufferTextureFormat::DEPTH_COMPONENT32F:
        return true;
    }

    return false;
}

static GLenum fermionFBTextureFormatToGL(FramebufferTextureFormat format) {
    switch (format) {
    case FramebufferTextureFormat::RGBA8:
        return GL_RGBA8;
    case FramebufferTextureFormat::RED_INTEGER:
        return GL_RED_INTEGER;
    case FramebufferTextureFormat::RGB16F:
        return GL_RGB16F;
    case FramebufferTextureFormat::RG16F:
        return GL_RG16F;
    }

    FERMION_ASSERT(false, "Invalid framebuffer texture format!");
    return 0;
}

static GLenum fermionFBBaseFormatToGL(FramebufferTextureFormat format) {
    switch (format) {
    case FramebufferTextureFormat::RGBA8:
        return GL_RGBA;
    case FramebufferTextureFormat::RED_INTEGER:
        return GL_RED_INTEGER;
    case FramebufferTextureFormat::RGB16F:
        return GL_RGB;
    case FramebufferTextureFormat::RG16F:
        return GL_RG;
    }
    FERMION_ASSERT(false, "Invalid framebuffer base format!");
    return 0;
}

} // namespace Utils

OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification &spec) : m_specification(spec) {
    for (auto spec : m_specification.attachments.attachments) {
        if (!Utils::isDepthFormat(spec.textureFormat))
            m_colorAttachmentSpecifications.emplace_back(spec);
        else
            m_depthAttachmentSpecification = spec;
    }

    invalidate();
}

OpenGLFramebuffer::~OpenGLFramebuffer() {
    glDeleteFramebuffers(1, &m_rendererID);
    glDeleteTextures((GLsizei)m_colorAttachments.size(), m_colorAttachments.data());
    glDeleteTextures(1, &m_depthAttachment);
}

void OpenGLFramebuffer::invalidate() {
    if (m_rendererID) {
        glDeleteFramebuffers(1, &m_rendererID);
        glDeleteTextures((GLsizei)m_colorAttachments.size(), m_colorAttachments.data());
        glDeleteTextures(1, &m_depthAttachment);

        m_colorAttachments.clear();
        m_depthAttachment = 0;
    }

    glCreateFramebuffers(1, &m_rendererID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);

    bool multisample = m_specification.samples > 1;

    // Attachments
    if (m_colorAttachmentSpecifications.size()) {
        m_colorAttachments.resize(m_colorAttachmentSpecifications.size());
        Utils::createTextures(multisample, m_colorAttachments.data(), (uint32_t)m_colorAttachments.size());

        for (size_t i = 0; i < m_colorAttachments.size(); i++) {
            Utils::bindTexture(multisample, m_colorAttachments[i]);
            switch (m_colorAttachmentSpecifications[i].textureFormat) {
            case FramebufferTextureFormat::RGBA8:
                Utils::attachColorTexture(m_colorAttachments[i], m_specification.samples, GL_RGBA8, GL_RGBA, m_specification.width, m_specification.height, (int)i);
                break;
            case FramebufferTextureFormat::RED_INTEGER:
                Utils::attachColorTexture(m_colorAttachments[i], m_specification.samples, GL_R32I, GL_RED_INTEGER, m_specification.width, m_specification.height, (int)i);
                break;
            case FramebufferTextureFormat::RGB16F:
                Utils::attachColorTexture(m_colorAttachments[i], m_specification.samples, GL_RGB16F, GL_RGB, m_specification.width, m_specification.height, (int)i);
                break;
            case FramebufferTextureFormat::RG16F:
                Utils::attachColorTexture(m_colorAttachments[i], m_specification.samples, GL_RG16F, GL_RG, m_specification.width, m_specification.height, (int)i);
                break;
            }
        }
    }

    if (m_depthAttachmentSpecification.textureFormat != FramebufferTextureFormat::None) {
        Utils::createTextures(multisample, &m_depthAttachment, 1);
        Utils::bindTexture(multisample, m_depthAttachment);
        switch (m_depthAttachmentSpecification.textureFormat) {
        case FramebufferTextureFormat::DEPTH24STENCIL8:
            Utils::attachDepthTexture(m_depthAttachment, m_specification.samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_specification.width, m_specification.height);
            break;
        case FramebufferTextureFormat::DEPTH_COMPONENT32F:
            Utils::attachDepthTexture(m_depthAttachment, m_specification.samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_specification.width, m_specification.height, true);
            break;
        }
    }

    if (m_colorAttachments.size() > 1) {
        FERMION_ASSERT(m_colorAttachments.size() <= 4, "Maximum number of color attachments exceeded!");
        GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers((GLsizei)m_colorAttachments.size(), buffers);
    } else if (m_colorAttachments.empty()) {
        // Only depth-pass
        glDrawBuffer(GL_NONE);
    }

    FERMION_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLFramebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);
    glViewport(0, 0, m_specification.width, m_specification.height);
}

void OpenGLFramebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLFramebuffer::resize(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize) {
        Log::Warn(std::format("Attempted to rezize framebuffer to {0}, {1}", width, height));
        return;
    }
    m_specification.width = width;
    m_specification.height = height;

    invalidate();
}

int OpenGLFramebuffer::readPixel(uint32_t attachmentIndex, int x, int y) {
    FERMION_ASSERT(attachmentIndex < m_colorAttachments.size(), "Attachment index out of range!");
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
    int pixelData;
    glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
    return pixelData;
}

void OpenGLFramebuffer::clearAttachment(uint32_t attachmentIndex, int value) {
    FERMION_ASSERT(attachmentIndex < m_colorAttachments.size(), "Attachment index out of range!");

    auto &spec = m_colorAttachmentSpecifications[attachmentIndex];
    if (spec.textureFormat == FramebufferTextureFormat::RED_INTEGER) {
        glClearTexImage(m_colorAttachments[attachmentIndex], 0,
                        Utils::fermionFBBaseFormatToGL(spec.textureFormat), GL_INT, &value);
    } else if (spec.textureFormat == FramebufferTextureFormat::RGBA8) {
        GLint clearColor[4] = {value, value, value, value};
        glClearTexImage(m_colorAttachments[attachmentIndex], 0,
                        Utils::fermionFBBaseFormatToGL(spec.textureFormat), GL_INT, clearColor);
    }
}

void OpenGLFramebuffer::bindColorAttachment(uint32_t attachmentIndex, uint32_t slot) const {
    FERMION_ASSERT(attachmentIndex < m_colorAttachments.size(), "Attachment index out of range!");
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(Utils::textureTarget(m_specification.samples > 1), m_colorAttachments[attachmentIndex]);
}

void OpenGLFramebuffer::bindDepthAttachment(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(Utils::textureTarget(m_specification.samples > 1), m_depthAttachment);
}

void OpenGLFramebuffer::blitTo(const std::shared_ptr<Framebuffer> &target, const FramebufferBlitSpecification &spec) const {
    auto targetFB = std::dynamic_pointer_cast<OpenGLFramebuffer>(target);
    FERMION_ASSERT(targetFB, "Target framebuffer must be OpenGLFramebuffer!");
    if (!targetFB)
        return;

    GLint prevRead = 0;
    GLint prevDraw = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_rendererID);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFB->m_rendererID);

    GLenum mask = 0;
    const bool blitColor = hasBlitMask(spec.mask, FramebufferBlitMask::Color);
    if (blitColor) {
        FERMION_ASSERT(spec.srcAttachmentIndex < m_colorAttachments.size(), "Source attachment index out of range!");
        FERMION_ASSERT(spec.dstAttachmentIndex < targetFB->m_colorAttachments.size(), "Destination attachment index out of range!");
        glReadBuffer(GL_COLOR_ATTACHMENT0 + spec.srcAttachmentIndex);
        glDrawBuffer(GL_COLOR_ATTACHMENT0 + spec.dstAttachmentIndex);
        mask |= GL_COLOR_BUFFER_BIT;
    } else {
        glReadBuffer(GL_NONE);
        glDrawBuffer(GL_NONE);
    }

    if (hasBlitMask(spec.mask, FramebufferBlitMask::Depth))
        mask |= GL_DEPTH_BUFFER_BIT;
    if (hasBlitMask(spec.mask, FramebufferBlitMask::Stencil))
        mask |= GL_STENCIL_BUFFER_BIT;

    GLenum filter = (spec.filter == FramebufferBlitFilter::Linear) ? GL_LINEAR : GL_NEAREST;
    if (blitColor && spec.srcAttachmentIndex < m_colorAttachmentSpecifications.size()) {
        const auto srcFormat = m_colorAttachmentSpecifications[spec.srcAttachmentIndex].textureFormat;
        if (srcFormat == FramebufferTextureFormat::RED_INTEGER)
            filter = GL_NEAREST;
    }

    glBlitFramebuffer(0, 0, m_specification.width, m_specification.height,
                      0, 0, targetFB->m_specification.width, targetFB->m_specification.height,
                      mask, filter);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_rendererID);
    if (m_colorAttachments.empty()) {
        glReadBuffer(GL_NONE);
    } else {
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFB->m_rendererID);
    if (targetFB->m_colorAttachments.empty()) {
        glDrawBuffer(GL_NONE);
    } else if (targetFB->m_colorAttachments.size() == 1) {
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
    } else {
        FERMION_ASSERT(targetFB->m_colorAttachments.size() <= 4, "Maximum number of color attachments exceeded!");
        GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers((GLsizei)targetFB->m_colorAttachments.size(), buffers);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, prevRead);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDraw);
}

} // namespace Fermion
