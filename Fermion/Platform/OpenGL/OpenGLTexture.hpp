#pragma once

#include "Renderer/Texture.hpp"

#include <glad/glad.h>

namespace Fermion
{

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const TextureSpecification &specification, bool generateMipmap = false);
		OpenGLTexture2D(uint32_t width, uint32_t height, bool generateMipmap = false);
		OpenGLTexture2D(const std::string &path, bool generateMipmap = false);
		virtual ~OpenGLTexture2D();

		virtual const TextureSpecification &getSpecification() const override { return m_specification; }

		virtual uint32_t getWidth() const override { return m_width; }
		virtual uint32_t getHeight() const override { return m_height; }
		virtual uint32_t getRendererID() const override { return m_rendererID; }

		virtual const std::string &getPath() const override { return m_path; }

		virtual void setData(void *data, uint32_t size) override;

		virtual void bind(uint32_t slot = 0) const override;

		virtual bool isLoaded() const override { return m_isLoaded; }

		virtual bool operator==(const Texture &other) const override
		{
			return m_rendererID == other.getRendererID();
		}

	private:
		TextureSpecification m_specification;

		std::string m_path;
		bool m_isLoaded = false;
		bool m_generateMipmap = true;
		uint32_t m_width, m_height;
		uint32_t m_rendererID;
		GLenum m_internalFormat, m_dataFormat;
	};

}
