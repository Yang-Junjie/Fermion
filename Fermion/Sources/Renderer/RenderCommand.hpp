#pragma once

#include "Renderer/RendererAPI.hpp"

namespace Fermion {

	class RenderCommand
	{
	public:
		static void init()
		{
			s_rendererAPI->init();
		}

		static void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_rendererAPI->setViewport(x, y, width, height);
		}

		static void setClearColor(const glm::vec4& color)
		{
			s_rendererAPI->setClearColor(color);
		}

		static void clear()
		{
			s_rendererAPI->clear();
		}

		static void drawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_rendererAPI->drawIndexed(vertexArray, indexCount);
		}

		static void drawLines(const std::shared_ptr<VertexArray>& vertexArray, uint32_t vertexCount)
		{
			s_rendererAPI->drawLines(vertexArray, vertexCount);
		}

		static void setLineWidth(float width)
		{
			s_rendererAPI->setLineWidth(width);
		}
	private:
		static std::unique_ptr<RendererAPI> s_rendererAPI;
	};

}
