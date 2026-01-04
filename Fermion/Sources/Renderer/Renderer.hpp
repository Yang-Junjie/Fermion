

// 暂时或可能以后都没用
#pragma once
#include "fmpch.hpp"
#include "RenderCommand.hpp"
#include "Camera/OrthographicCamera.hpp"
#include "Shader.hpp"
#include "RendererConfig.hpp"
namespace Fermion {

class Renderer {
public:
    static void init();
    static void setConfig(const RendererConfig &config);
    static void shutdown();

    static void onWindowResize(uint32_t width, uint32_t height);

    static void beginScene(OrthographicCamera &camera);
    static void endScene();

    static const ShaderLibrary* getShaderLibrary() {
        return s_shaderLibrary.get();
    }

    static void submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray, const glm::mat4 &transform = glm::mat4(1.0f));

    static RendererAPI::API getAPI() {
        return RendererAPI::getAPI();
    }

private:
    struct SceneData {
        glm::mat4 viewProjectionMatrix;
    };
    static std::unique_ptr<SceneData> s_sceneData;
    inline static std::unique_ptr<ShaderLibrary> s_shaderLibrary;
    inline static RendererConfig s_config;
};
} // namespace Fermion
