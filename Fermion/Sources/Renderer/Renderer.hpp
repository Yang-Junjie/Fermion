#pragma once
#include "fmpch.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/OrthographicCamera.hpp"
#include "Renderer/Shader.hpp"
namespace Fermion
{

	class Renderer
	{
	public:
		static void init();
		static void shutdown();

		static void onWindowResize(uint32_t width, uint32_t height);

		static void beginScene(OrthographicCamera &camera);
		static void endScene();

		static void submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray,const glm::mat4& transform = glm::mat4(1.0f));

		static RendererAPI::API getAPI() { return RendererAPI::getAPI(); }

	private:
		struct SceneData
		{
			glm::mat4 viewProjectionMatrix;
		};
		static std::unique_ptr<SceneData> s_sceneData;
	};
}
