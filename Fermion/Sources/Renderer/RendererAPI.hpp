#pragma once

#include "Renderer/VertexArray.hpp"

#include <glm/glm.hpp>

namespace Fermion {

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1
		};
	public:
		virtual ~RendererAPI() = default;

		virtual void init() = 0;
		virtual void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void setClearColor(const glm::vec4& color) = 0;
		virtual void clear() = 0;

		virtual void drawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void drawLines(const std::shared_ptr<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
		
		virtual void setLineWidth(float width) = 0;

		static API getAPI() { return s_API; }
		static std::unique_ptr<RendererAPI> create();
	private:
		static API s_API;
	};

}
