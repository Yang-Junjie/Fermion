

// 暂时或可能以后都没用
#pragma once
#include "fmpch.hpp"
#include "Renderer/RendererAPI.hpp"
#include "Renderer/Camera/OrthographicCamera.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/RendererConfig.hpp"
namespace Fermion {

class Renderer {
public:
    static void init();
    static void setConfig(const RendererConfig &config);
    static void shutdown();

    static void onWindowResize(uint32_t width, uint32_t height);

    // static void beginScene(OrthographicCamera &camera);
    // static void endScene();

    static const ShaderLibrary* getShaderLibrary() {
        return s_shaderLibrary.get();
    }

    static void submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray, const glm::mat4 &transform = glm::mat4(1.0f));

    static RendererAPI::API getAPI() {
        return RendererAPI::getAPI();
    }

    static RendererAPI& getRendererAPI() {
        return *s_rendererAPI;
    }

private:
    inline static std::unique_ptr<RendererAPI> s_rendererAPI;
    struct SceneData {
        glm::mat4 viewProjectionMatrix;
    };
    static std::unique_ptr<SceneData> s_sceneData;
    inline static std::unique_ptr<ShaderLibrary> s_shaderLibrary;
    inline static RendererConfig s_config;
};
} // namespace Fermion
