#include "fmpch.hpp"
#include "OpenGLFramebuffer.hpp"

#include <glad/glad.h>

namespace Fermion
{

    static const uint32_t s_MaxFramebufferSize = 8192;

    namespace Utils
    {

        static GLenum textureTarget(bool multisampled)
        {
            return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        }

        static void createTextures(bool multisampled, uint32_t *outID, uint32_t count)
        {
            glCreateTextures(textureTarget(multisampled), count, outID);
        }

        static void bindTexture(bool multisampled, uint32_t id)
        {
            glBindTexture(textureTarget(multisampled), id);
        }

        static void attachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index)
        {
            bool multisampled = samples > 1;
            if (multisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
            }
            else
            {
                // Choose data type based on format
                GLenum type = GL_UNSIGNED_BYTE;
                if (format == GL_RED_INTEGER)
                {
                    type = GL_INT;
                }
                else if (internalFormat == GL_RGB16F || internalFormat == GL_RG16F || internalFormat == GL_RGBA16F)
                {
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

        static void attachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height, bool isShadowMap = false)
        {
            bool multisampled = samples > 1;
            if (multisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
            }
            else
            {
                glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

                if (isShadowMap)
                {
                    // Shadow map specific settings
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
                    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                }
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, textureTarget(multisampled), id, 0);
        }

        static bool isDepthFormat(FramebufferTextureFormat format)
        {
            switch (format)
            {
            case FramebufferTextureFormat::DEPTH24STENCIL8:
            case FramebufferTextureFormat::DEPTH_COMPONENT32F:
                return true;
            }

            return false;
        }

        static GLenum fermionFBTextureFormatToGL(FramebufferTextureFormat format)
        {
            switch (format)
            {
            case FramebufferTextureFormat::RGBA8:
                return GL_RGBA8;
            case FramebufferTextureFormat::RED_INTEGER:
                return GL_RED_INTEGER;
            case FramebufferTextureFormat::RGB16F:
                return GL_RGB16F;
            case FramebufferTextureFormat::RGBA16F:
                return GL_RGBA16F;
            case FramebufferTextureFormat::RG16F:
                return GL_RG16F;
            }

            FERMION_ASSERT(false, "Invalid framebuffer texture format!");
            return 0;
        }

        static GLenum fermionFBBaseFormatToGL(FramebufferTextureFormat format)
        {
            switch (format)
            {
            case FramebufferTextureFormat::RGBA8:
                return GL_RGBA;
            case FramebufferTextureFormat::RED_INTEGER:
                return GL_RED_INTEGER;
            case FramebufferTextureFormat::RGB16F:
                return GL_RGB;
            case FramebufferTextureFormat::RGBA16F:
                return GL_RGBA;
            case FramebufferTextureFormat::RG16F:
                return GL_RG;
            }
            FERMION_ASSERT(false, "Invalid framebuffer base format!");
            return 0;
        }

    } // namespace Utils

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification &spec) : m_specification(spec)
    {
        for (auto spec : m_specification.attachments.attachments)
        {
            if (!Utils::isDepthFormat(spec.textureFormat))
                m_colorAttachmentSpecifications.emplace_back(spec);
            else
                m_depthAttachmentSpecification = spec;
        }

        invalidate();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        glDeleteFramebuffers(1, &m_rendererID);
        glDeleteTextures((GLsizei)m_colorAttachments.size(), m_colorAttachments.data());
        glDeleteTextures(1, &m_depthAttachment);

        // Clean up resolve framebuffer
        if (m_resolveRendererID)
        {
            glDeleteFramebuffers(1, &m_resolveRendererID);
            glDeleteTextures((GLsizei)m_resolveColorAttachments.size(), m_resolveColorAttachments.data());
            glDeleteTextures(1, &m_resolveDepthAttachment);
        }
    }

    void OpenGLFramebuffer::invalidate()
    {
        if (m_rendererID)
        {
            glDeleteFramebuffers(1, &m_rendererID);
            glDeleteTextures((GLsizei)m_colorAttachments.size(), m_colorAttachments.data());
            glDeleteTextures(1, &m_depthAttachment);

            m_colorAttachments.clear();
            m_depthAttachment = 0;

            // Clean up resolve framebuffer
            if (m_resolveRendererID)
            {
                glDeleteFramebuffers(1, &m_resolveRendererID);
                glDeleteTextures((GLsizei)m_resolveColorAttachments.size(), m_resolveColorAttachments.data());
                glDeleteTextures(1, &m_resolveDepthAttachment);
                m_resolveRendererID = 0;
                m_resolveColorAttachments.clear();
                m_resolveDepthAttachment = 0;
            }
        }

        glCreateFramebuffers(1, &m_rendererID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);

        bool multisample = m_specification.samples > 1;

        // Attachments
        if (m_colorAttachmentSpecifications.size())
        {
            m_colorAttachments.resize(m_colorAttachmentSpecifications.size());
            Utils::createTextures(multisample, m_colorAttachments.data(), (uint32_t)m_colorAttachments.size());

            for (size_t i = 0; i < m_colorAttachments.size(); i++)
            {
                Utils::bindTexture(multisample, m_colorAttachments[i]);
                switch (m_colorAttachmentSpecifications[i].textureFormat)
                {
                case FramebufferTextureFormat::RGBA8:
                    Utils::attachColorTexture(m_colorAttachments[i], m_specification.samples, GL_RGBA8, GL_RGBA, m_specification.width, m_specification.height, (int)i);
                    break;
                case FramebufferTextureFormat::RED_INTEGER:
                    Utils::attachColorTexture(m_colorAttachments[i], m_specification.samples, GL_R32I, GL_RED_INTEGER, m_specification.width, m_specification.height, (int)i);
                    break;
                case FramebufferTextureFormat::RGB16F:
                    Utils::attachColorTexture(m_colorAttachments[i], m_specification.samples, GL_RGB16F, GL_RGB, m_specification.width, m_specification.height, (int)i);
                    break;
                case FramebufferTextureFormat::RGBA16F:
                    Utils::attachColorTexture(m_colorAttachments[i], m_specification.samples, GL_RGBA16F, GL_RGBA, m_specification.width, m_specification.height, (int)i);
                    break;
                case FramebufferTextureFormat::RG16F:
                    Utils::attachColorTexture(m_colorAttachments[i], m_specification.samples, GL_RG16F, GL_RG, m_specification.width, m_specification.height, (int)i);
                    break;
                }
            }
        }

        if (m_depthAttachmentSpecification.textureFormat != FramebufferTextureFormat::None)
        {
            Utils::createTextures(multisample, &m_depthAttachment, 1);
            Utils::bindTexture(multisample, m_depthAttachment);
            switch (m_depthAttachmentSpecification.textureFormat)
            {
            case FramebufferTextureFormat::DEPTH24STENCIL8:
                Utils::attachDepthTexture(m_depthAttachment, m_specification.samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_specification.width, m_specification.height);
                break;
            case FramebufferTextureFormat::DEPTH_COMPONENT32F:
                Utils::attachDepthTexture(m_depthAttachment, m_specification.samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_specification.width, m_specification.height, true);
                break;
            }
        }

        if (m_colorAttachments.size() > 1)
        {
            GLint maxAttachments = 0;
            glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachments);
            FERMION_ASSERT(m_colorAttachments.size() <= static_cast<size_t>(maxAttachments),
                           "Maximum number of color attachments exceeded!");
            std::vector<GLenum> buffers;
            buffers.reserve(m_colorAttachments.size());
            for (size_t i = 0; i < m_colorAttachments.size(); ++i)
                buffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i));
            glDrawBuffers((GLsizei)m_colorAttachments.size(), buffers.data());
        }
        else if (m_colorAttachments.empty())
        {
            // Only depth-pass
            glDrawBuffer(GL_NONE);
        }

        FERMION_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

        // Create resolve framebuffer for MSAA
        if (multisample)
        {
            glCreateFramebuffers(1, &m_resolveRendererID);
            glBindFramebuffer(GL_FRAMEBUFFER, m_resolveRendererID);

            // Create resolve color attachments (non-multisampled)
            if (m_colorAttachmentSpecifications.size())
            {
                m_resolveColorAttachments.resize(m_colorAttachmentSpecifications.size());
                Utils::createTextures(false, m_resolveColorAttachments.data(), (uint32_t)m_resolveColorAttachments.size());

                for (size_t i = 0; i < m_resolveColorAttachments.size(); i++)
                {
                    Utils::bindTexture(false, m_resolveColorAttachments[i]);
                    switch (m_colorAttachmentSpecifications[i].textureFormat)
                    {
                    case FramebufferTextureFormat::RGBA8:
                        Utils::attachColorTexture(m_resolveColorAttachments[i], 1, GL_RGBA8, GL_RGBA, m_specification.width, m_specification.height, (int)i);
                        break;
                    case FramebufferTextureFormat::RED_INTEGER:
                        Utils::attachColorTexture(m_resolveColorAttachments[i], 1, GL_R32I, GL_RED_INTEGER, m_specification.width, m_specification.height, (int)i);
                        break;
                    case FramebufferTextureFormat::RGB16F:
                        Utils::attachColorTexture(m_resolveColorAttachments[i], 1, GL_RGB16F, GL_RGB, m_specification.width, m_specification.height, (int)i);
                        break;
                    case FramebufferTextureFormat::RGBA16F:
                        Utils::attachColorTexture(m_resolveColorAttachments[i], 1, GL_RGBA16F, GL_RGBA, m_specification.width, m_specification.height, (int)i);
                        break;
                    case FramebufferTextureFormat::RG16F:
                        Utils::attachColorTexture(m_resolveColorAttachments[i], 1, GL_RG16F, GL_RG, m_specification.width, m_specification.height, (int)i);
                        break;
                    }
                }
            }

            // Create resolve depth attachment (non-multisampled)
            if (m_depthAttachmentSpecification.textureFormat != FramebufferTextureFormat::None)
            {
                Utils::createTextures(false, &m_resolveDepthAttachment, 1);
                Utils::bindTexture(false, m_resolveDepthAttachment);
                switch (m_depthAttachmentSpecification.textureFormat)
                {
                case FramebufferTextureFormat::DEPTH24STENCIL8:
                    Utils::attachDepthTexture(m_resolveDepthAttachment, 1, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_specification.width, m_specification.height);
                    break;
                case FramebufferTextureFormat::DEPTH_COMPONENT32F:
                    Utils::attachDepthTexture(m_resolveDepthAttachment, 1, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_specification.width, m_specification.height, true);
                    break;
                }
            }

            // Set up draw buffers for resolve FBO
            if (m_resolveColorAttachments.size() > 1)
            {
                std::vector<GLenum> buffers;
                buffers.reserve(m_resolveColorAttachments.size());
                for (size_t i = 0; i < m_resolveColorAttachments.size(); ++i)
                    buffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i));
                glDrawBuffers((GLsizei)m_resolveColorAttachments.size(), buffers.data());
            }
            else if (m_resolveColorAttachments.empty())
            {
                glDrawBuffer(GL_NONE);
            }

            FERMION_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Resolve framebuffer is incomplete!");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);
        glViewport(0, 0, m_specification.width, m_specification.height);

        // Enable multisampling for MSAA framebuffers
        if (isMultisampled())
        {
            glEnable(GL_MULTISAMPLE);
        }
    }

    void OpenGLFramebuffer::unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::bindForRead()
    {
        // For MSAA, bind the resolve FBO (must call resolve() first)
        if (isMultisampled() && m_resolveRendererID)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_resolveRendererID);
            glViewport(0, 0, m_specification.width, m_specification.height);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);
            glViewport(0, 0, m_specification.width, m_specification.height);
        }
    }

    void OpenGLFramebuffer::resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
        {
            Log::Warn(std::format("Attempted to rezize framebuffer to {0}, {1}", width, height));
            return;
        }
        m_specification.width = width;
        m_specification.height = height;

        invalidate();
    }

    void OpenGLFramebuffer::resolve()
    {
        if (!isMultisampled() || !m_resolveRendererID)
            return;

        GLint prevRead = 0;
        GLint prevDraw = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_rendererID);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolveRendererID);

        // Resolve each color attachment
        // Note: For MSAA resolve, filter is effectively ignored as GPU performs box filter
        // But GL_NEAREST is required for integer formats
        for (size_t i = 0; i < m_colorAttachments.size(); ++i)
        {
            glReadBuffer(GL_COLOR_ATTACHMENT0 + (GLenum)i);
            glDrawBuffer(GL_COLOR_ATTACHMENT0 + (GLenum)i);

            glBlitFramebuffer(0, 0, m_specification.width, m_specification.height,
                              0, 0, m_specification.width, m_specification.height,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        // Resolve depth/stencil attachment
        if (m_depthAttachment && m_resolveDepthAttachment)
        {
            GLenum depthMask = GL_DEPTH_BUFFER_BIT;
            if (m_depthAttachmentSpecification.textureFormat == FramebufferTextureFormat::DEPTH24STENCIL8)
            {
                depthMask |= GL_STENCIL_BUFFER_BIT;
            }
            glBlitFramebuffer(0, 0, m_specification.width, m_specification.height,
                              0, 0, m_specification.width, m_specification.height,
                              depthMask, GL_NEAREST);
        }

        // Restore draw buffers for resolve FBO
        if (m_resolveColorAttachments.size() > 1)
        {
            std::vector<GLenum> buffers;
            buffers.reserve(m_resolveColorAttachments.size());
            for (size_t i = 0; i < m_resolveColorAttachments.size(); ++i)
                buffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i));
            glDrawBuffers((GLsizei)m_resolveColorAttachments.size(), buffers.data());
        }
        else if (!m_resolveColorAttachments.empty())
        {
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, prevRead);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDraw);
    }

    int OpenGLFramebuffer::readPixel(uint32_t attachmentIndex, int x, int y)
    {
        FERMION_ASSERT(attachmentIndex < m_colorAttachments.size(), "Attachment index out of range!");

        // For MSAA, resolve first and read from resolve FBO
        if (isMultisampled())
        {
            resolve();
        }

        uint32_t fboToRead = isMultisampled() ? m_resolveRendererID : m_rendererID;

        GLint prevReadFramebuffer = 0;
        GLint prevReadBuffer = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFramebuffer);
        glGetIntegerv(GL_READ_BUFFER, &prevReadBuffer);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboToRead);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
        int pixelData = -1;
        glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);

        glReadBuffer(prevReadBuffer);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFramebuffer);
        return pixelData;
    }

    void OpenGLFramebuffer::clearAttachment(uint32_t attachmentIndex, int value)
    {
        FERMION_ASSERT(attachmentIndex < m_colorAttachments.size(), "Attachment index out of range!");

        auto &spec = m_colorAttachmentSpecifications[attachmentIndex];

        // For MSAA, we need to use glClearBuffer* instead of glClearTexImage
        if (isMultisampled())
        {
            GLint prevFramebuffer = 0;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);

            if (spec.textureFormat == FramebufferTextureFormat::RED_INTEGER)
            {
                GLint clearValue = value;
                glClearBufferiv(GL_COLOR, attachmentIndex, &clearValue);
            }
            else if (spec.textureFormat == FramebufferTextureFormat::RGBA8)
            {
                GLfloat clearColor[4] = {value / 255.0f, value / 255.0f, value / 255.0f, value / 255.0f};
                glClearBufferfv(GL_COLOR, attachmentIndex, clearColor);
            }
            else if (spec.textureFormat == FramebufferTextureFormat::RGB16F ||
                     spec.textureFormat == FramebufferTextureFormat::RGBA16F ||
                     spec.textureFormat == FramebufferTextureFormat::RG16F)
            {
                GLfloat clearColor[4] = {static_cast<float>(value), static_cast<float>(value),
                                         static_cast<float>(value), static_cast<float>(value)};
                glClearBufferfv(GL_COLOR, attachmentIndex, clearColor);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, prevFramebuffer);
        }
        else
        {
            if (spec.textureFormat == FramebufferTextureFormat::RED_INTEGER)
            {
                glClearTexImage(m_colorAttachments[attachmentIndex], 0,
                                Utils::fermionFBBaseFormatToGL(spec.textureFormat), GL_INT, &value);
            }
            else if (spec.textureFormat == FramebufferTextureFormat::RGBA8)
            {
                GLint clearColor[4] = {value, value, value, value};
                glClearTexImage(m_colorAttachments[attachmentIndex], 0,
                                Utils::fermionFBBaseFormatToGL(spec.textureFormat), GL_INT, clearColor);
            }
            else if (spec.textureFormat == FramebufferTextureFormat::RGB16F ||
                     spec.textureFormat == FramebufferTextureFormat::RGBA16F ||
                     spec.textureFormat == FramebufferTextureFormat::RG16F)
            {
                float clearColor[4] = {static_cast<float>(value),
                                       static_cast<float>(value),
                                       static_cast<float>(value),
                                       static_cast<float>(value)};
                glClearTexImage(m_colorAttachments[attachmentIndex], 0,
                                Utils::fermionFBBaseFormatToGL(spec.textureFormat), GL_FLOAT, clearColor);
            }
        }
    }

    void OpenGLFramebuffer::bindColorAttachment(uint32_t attachmentIndex, uint32_t slot) const
    {
        FERMION_ASSERT(attachmentIndex < m_colorAttachments.size(), "Attachment index out of range!");
        glActiveTexture(GL_TEXTURE0 + slot);

        // For MSAA, bind the resolved texture (must call resolve() first)
        if (isMultisampled() && attachmentIndex < m_resolveColorAttachments.size())
        {
            glBindTexture(GL_TEXTURE_2D, m_resolveColorAttachments[attachmentIndex]);
        }
        else
        {
            glBindTexture(Utils::textureTarget(m_specification.samples > 1), m_colorAttachments[attachmentIndex]);
        }
    }

    void OpenGLFramebuffer::bindDepthAttachment(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);

        // For MSAA, bind the resolved depth texture (must call resolve() first)
        if (isMultisampled() && m_resolveDepthAttachment)
        {
            glBindTexture(GL_TEXTURE_2D, m_resolveDepthAttachment);
        }
        else
        {
            glBindTexture(Utils::textureTarget(m_specification.samples > 1), m_depthAttachment);
        }
    }

    void OpenGLFramebuffer::blitTo(const std::shared_ptr<Framebuffer> &target, const FramebufferBlitSpecification &spec) const
    {
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
        if (blitColor)
        {
            FERMION_ASSERT(spec.srcAttachmentIndex < m_colorAttachments.size(), "Source attachment index out of range!");
            FERMION_ASSERT(spec.dstAttachmentIndex < targetFB->m_colorAttachments.size(), "Destination attachment index out of range!");
            glReadBuffer(GL_COLOR_ATTACHMENT0 + spec.srcAttachmentIndex);
            glDrawBuffer(GL_COLOR_ATTACHMENT0 + spec.dstAttachmentIndex);
            mask |= GL_COLOR_BUFFER_BIT;
        }
        else
        {
            glReadBuffer(GL_NONE);
            glDrawBuffer(GL_NONE);
        }

        if (hasBlitMask(spec.mask, FramebufferBlitMask::Depth))
            mask |= GL_DEPTH_BUFFER_BIT;
        if (hasBlitMask(spec.mask, FramebufferBlitMask::Stencil))
            mask |= GL_STENCIL_BUFFER_BIT;

        GLenum filter = (spec.filter == FramebufferBlitFilter::Linear) ? GL_LINEAR : GL_NEAREST;
        if (blitColor && spec.srcAttachmentIndex < m_colorAttachmentSpecifications.size())
        {
            const auto srcFormat = m_colorAttachmentSpecifications[spec.srcAttachmentIndex].textureFormat;
            if (srcFormat == FramebufferTextureFormat::RED_INTEGER)
                filter = GL_NEAREST;
        }

        glBlitFramebuffer(0, 0, m_specification.width, m_specification.height,
                          0, 0, targetFB->m_specification.width, targetFB->m_specification.height,
                          mask, filter);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_rendererID);
        if (m_colorAttachments.empty())
        {
            glReadBuffer(GL_NONE);
        }
        else
        {
            glReadBuffer(GL_COLOR_ATTACHMENT0);
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFB->m_rendererID);
        if (targetFB->m_colorAttachments.empty())
        {
            glDrawBuffer(GL_NONE);
        }
        else if (targetFB->m_colorAttachments.size() == 1)
        {
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
        }
        else
        {
            GLint maxAttachments = 0;
            glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachments);
            FERMION_ASSERT(targetFB->m_colorAttachments.size() <= static_cast<size_t>(maxAttachments),
                           "Maximum number of color attachments exceeded!");
            std::vector<GLenum> buffers;
            buffers.reserve(targetFB->m_colorAttachments.size());
            for (size_t i = 0; i < targetFB->m_colorAttachments.size(); ++i)
                buffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i));
            glDrawBuffers((GLsizei)targetFB->m_colorAttachments.size(), buffers.data());
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, prevRead);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDraw);
    }

} // namespace Fermion
