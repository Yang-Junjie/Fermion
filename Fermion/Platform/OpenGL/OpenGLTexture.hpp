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
		virtual uint32_t getWidth() const override { return m_Width; }
		virtual uint32_t getHeight() const override { return m_Height; }

	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
	};

}
