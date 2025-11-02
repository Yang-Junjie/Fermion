#pragma once

#include "Renderer/Texture.hpp"

#include <glad/glad.h>

namespace Fermion
{

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string &path);
		virtual ~OpenGLTexture2D();

		virtual void bind(uint32_t slot = 0) const override;
		virtual uint32_t getWidth() const override { return m_width; }
		virtual uint32_t getHeight() const override { return m_height; }

	private:
		std::string m_path;
		uint32_t m_width, m_height;
		uint32_t m_rendererID;
		GLenum m_internalFormat, m_dataFormat;
		bool m_isLoaded = false;
	
	};

}
