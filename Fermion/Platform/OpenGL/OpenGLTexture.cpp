#include "fmpch.hpp"
#include "OpenGLTexture.hpp"

#include <stb_image.h>

namespace Fermion
{

	OpenGLTexture2D::OpenGLTexture2D(const std::string &path)
		: m_Path(path)
	{

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc *data = nullptr;
		{
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		if (data)
		{
			Fermion::Log::Info("Loaded texture: " + path);
			m_Width = width;
			m_Height = height;

			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, 1, GL_RGB8, m_Width, m_Height);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}else{
			Fermion::Log::Error("Failed to load texture image: " + path);
		}
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{

		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::bind(uint32_t slot) const
	{

		glBindTextureUnit(slot, m_RendererID);
	}
}
