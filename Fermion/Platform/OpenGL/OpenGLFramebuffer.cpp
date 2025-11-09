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
				// Choose data type and filtering based on format (integer vs normalized)
				GLenum type = (format == GL_RED_INTEGER) ? GL_INT : GL_UNSIGNED_BYTE;
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

		static void attachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
		{
			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, textureTarget(multisampled), id, 0);
		}

		static bool isDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
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
			}

			FMAssert::Assert(false, "Invalid framebuffer texture format!", __FILE__, __LINE__);
			return 0;
		}

		// Returns the base format for glClearTexImage / glReadPixels
		static GLenum fermionFBBaseFormatToGL(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::RGBA8:
				return GL_RGBA;
			case FramebufferTextureFormat::RED_INTEGER:
				return GL_RED_INTEGER;
			}
			FMAssert::Assert(false, "Invalid framebuffer base format!", __FILE__, __LINE__);
			return 0;
		}

	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification &spec)
		: m_specification(spec)
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
			}
		}

		if (m_colorAttachments.size() > 1)
		{
			FMAssert::Assert(m_colorAttachments.size() <= 4, "Maximum number of color attachments exceeded!", __FILE__, __LINE__);
			GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
			glDrawBuffers((GLsizei)m_colorAttachments.size(), buffers);
		}
		else if (m_colorAttachments.empty())
		{
			// Only depth-pass
			glDrawBuffer(GL_NONE);
		}

		FMAssert::Assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!", __FILE__, __LINE__);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);
		glViewport(0, 0, m_specification.width, m_specification.height);
	}

	void OpenGLFramebuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	int OpenGLFramebuffer::readPixel(uint32_t attachmentIndex, int x, int y)
	{
		FMAssert::Assert(attachmentIndex < m_colorAttachments.size(), "Attachment index out of range!", __FILE__, __LINE__);

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixelData;
		// Assumes reading from an integer attachment (e.g., RED_INTEGER picking buffer)
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;
	}

	void OpenGLFramebuffer::clearAttachment(uint32_t attachmentIndex, int value)
	{

		FMAssert::Assert(attachmentIndex < m_colorAttachments.size(), "Attachment index out of range!", __FILE__, __LINE__);

		auto &spec = m_colorAttachmentSpecifications[attachmentIndex];
		if (spec.textureFormat == FramebufferTextureFormat::RED_INTEGER)
		{
			glClearTexImage(m_colorAttachments[attachmentIndex], 0,
							Utils::fermionFBBaseFormatToGL(spec.textureFormat), GL_INT, &value);
		}
		else if (spec.textureFormat == FramebufferTextureFormat::RGBA8)
		{
			GLint clearColor[4] = { value, value, value, value };
			glClearTexImage(m_colorAttachments[attachmentIndex], 0,
							Utils::fermionFBBaseFormatToGL(spec.textureFormat), GL_INT, clearColor);
		}
	}

}