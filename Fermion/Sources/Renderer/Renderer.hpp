#pragma once
#include "Renderer/RenderCommand.hpp"


namespace Fermion {

	class Renderer
	{
	public:
		static void init();
		static void shutdown();
		
		static void onWindowResize(uint32_t width, uint32_t height);

		static void beginScene();
		static void endScene();

		static void submit(const std::shared_ptr<VertexArray>& vertexArray);

		static RendererAPI::API getAPI() { return RendererAPI::getAPI(); }
	private:
	};
}
