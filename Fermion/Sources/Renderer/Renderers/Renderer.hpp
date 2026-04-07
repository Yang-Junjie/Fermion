#pragma once
#include "fmpch.hpp"
#include "Renderer/Camera/OrthographicCamera.hpp"
#include "Renderer/RendererAPI.hpp"
#include "Renderer/Shader.hpp"

namespace Fermion {
struct RendererConfig {
    RendererAPI::API Backend = RendererAPI::API::OpenGL;
    std::string ShaderPath;
};

class Renderer {
public:
    static void init();
    static void setConfig(const RendererConfig& config);
    static void shutdown();

    static void onWindowResize(uint32_t width, uint32_t height);

    static void submit(const std::shared_ptr<Shader>& shader,
                       const std::shared_ptr<VertexArray>& vertexArray,
                       const glm::mat4& transform = glm::mat4(1.0f));

    static const ShaderLibrary* getShaderLibrary()
    {
        return s_shaderLibrary.get();
    }

    static RendererAPI::API getAPI()
    {
        return RendererAPI::getAPI();
    }

    static RendererAPI& getRendererAPI()
    {
        return *s_rendererAPI;
    }

    static const RendererConfig& getConfig()
    {
        return s_config;
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
