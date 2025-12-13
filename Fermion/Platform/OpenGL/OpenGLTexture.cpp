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

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification &specification, bool generateMipmap)
		: m_specification(specification), m_width(m_specification.Width), m_height(m_specification.Height), m_generateMipmap(generateMipmap)
	{
		m_internalFormat = Utils::fermionImageFormatToGLInternalFormat(m_specification.Format);
		m_dataFormat = Utils::fermionImageFormatToGLDataFormat(m_specification.Format);

		int levels = generateMipmap ? 1 + (int)std::floor(std::log2(std::max(m_width, m_height))) : 1;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
		glTextureStorage2D(m_rendererID, levels, m_internalFormat, m_width, m_height);

		glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, generateMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (generateMipmap)
			glGenerateTextureMipmap(m_rendererID);
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, bool generateMipmap)
		: m_width(width), m_height(height), m_generateMipmap(generateMipmap)
	{
		m_internalFormat = GL_RGBA8;
		m_dataFormat = GL_RGBA;

		int levels = generateMipmap ? 1 + (int)std::floor(std::log2(std::max(m_width, m_height))) : 1;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
		glTextureStorage2D(m_rendererID, levels, m_internalFormat, m_width, m_height);

		glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, generateMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (generateMipmap)
			glGenerateTextureMipmap(m_rendererID);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string &path, bool generateMipmap)
		: m_path(path), m_generateMipmap(generateMipmap)
	{
		int width, height, channels;


		stbi_set_flip_vertically_on_load(1);

		stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 4);
		if (!data)
		{
			Log::Error(std::format("Failed to load texture image: {}", path));
			return;
		}

		m_isLoaded = true;
		m_width = width;
		m_height = height;
		m_internalFormat = GL_RGBA8;
		m_dataFormat = GL_RGBA;

		int levels = generateMipmap ? 1 + (int)std::floor(std::log2(std::max(m_width, m_height))) : 1;


		glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
		glTextureStorage2D(m_rendererID, levels, m_internalFormat, m_width, m_height);

		GLint previousAlignment;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTextureSubImage2D(
			m_rendererID, 0,
			0, 0,
			m_width, m_height,
			m_dataFormat,
			GL_UNSIGNED_BYTE,
			data);

		glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);


		glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, generateMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);


		if (generateMipmap)
			glGenerateTextureMipmap(m_rendererID);

		stbi_image_free(data);
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

		GLint previousAlignment;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTextureSubImage2D(
			m_rendererID, 0,
			0, 0,
			m_width, m_height,
			m_dataFormat,
			GL_UNSIGNED_BYTE,
			data);

		glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
	}

	void OpenGLTexture2D::bind(uint32_t slot) const
	{
		FM_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_rendererID);
	}
}
