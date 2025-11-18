#include "fmpch.hpp"
#include "OpenGLTexture.hpp"

#include <stb_image.h>

namespace Fermion
{
	namespace Utils
	{

		static GLenum fermionImageFormatToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8:
				return GL_RGB;
			case ImageFormat::RGBA8:
				return GL_RGBA;
			}

			FERMION_ASSERT(false, "Unknown ImageFormat!");
			return 0;
		}

		static GLenum fermionImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8:
				return GL_RGB8;
			case ImageFormat::RGBA8:
				return GL_RGBA8;
			}

			FERMION_ASSERT(false, "Unknown ImageFormat!");
			return 0;
		}

	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification &specification)
		: m_specification(specification), m_width(m_specification.Width), m_height(m_specification.Height)
	{
		FM_PROFILE_FUNCTION();

		m_internalFormat = Utils::fermionImageFormatToGLInternalFormat(m_specification.Format);
		m_dataFormat = Utils::fermionImageFormatToGLDataFormat(m_specification.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
		glTextureStorage2D(m_rendererID, 1, m_internalFormat, m_width, m_height);

		glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_width(width), m_height(height)
	{
		FM_PROFILE_FUNCTION();

		m_internalFormat = GL_RGBA8;
		m_dataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
		glTextureStorage2D(m_rendererID, 1, m_internalFormat, m_width, m_height);

		glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string &path)
		: m_path(path)
	{
		FM_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc *data = nullptr;
		{
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		if (data)
		{
			Log::Info(std::format("Loaded texture:{}", path));

			m_isLoaded = true;

			m_width = width;
			m_height = height;

			GLenum internalFormat = 0, dataFormat = 0;
			if (channels == 4)
			{
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
			}
			else if (channels == 3)
			{
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
			}

			m_internalFormat = internalFormat;
			m_dataFormat = dataFormat;

			glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
			glTextureStorage2D(m_rendererID, 1, internalFormat, m_width, m_height);

			glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, dataFormat, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else
		{
			Log::Error(std::format("Failed to load texture image: {}", path));
		}
	}
	OpenGLTexture2D::~OpenGLTexture2D()
	{
		FM_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_rendererID);
	}

	void OpenGLTexture2D::setData(void *data, uint32_t size)
	{
		FM_PROFILE_FUNCTION();

		uint32_t bpp = m_dataFormat == GL_RGBA ? 4 : 3;
		FERMION_ASSERT(size == m_width * m_height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, m_dataFormat, GL_UNSIGNED_BYTE, data);
	}
	void OpenGLTexture2D::bind(uint32_t slot) const
	{
		FM_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_rendererID);
	}
}
