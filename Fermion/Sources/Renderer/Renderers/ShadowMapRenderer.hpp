#pragma once
#include <memory>
#include <span>
#include <vector>

#include <glm/glm.hpp>

#include "Renderer/RenderPassQueue.hpp"
#include "Scene/Scene.hpp"

namespace Fermion
{
    class Framebuffer;
    struct MeshDrawCommand;
    class Pipeline;
    class UniformBuffer;

    class ShadowMapRenderer
    {
    public:
        ShadowMapRenderer();

        void addPass(RenderPassQueue &passQueue,
                     ResourceHandle shadowMap,
                     std::span<const MeshDrawCommand> drawList,
                     const DirectionalLight &light,
                     uint32_t shadowMapSize,
                     const std::shared_ptr<Framebuffer> &targetFramebuffer,
                     uint32_t viewportWidth,
                     uint32_t viewportHeight,
                     uint32_t *shadowDrawCalls,
                     const std::shared_ptr<UniformBuffer> &modelUniformBuffer,
                     const std::shared_ptr<UniformBuffer> &lightUniformBuffer,
                     const std::shared_ptr<UniformBuffer> &boneUniformBuffer = nullptr);

        const glm::mat4 &getLightSpaceMatrix() const;
        std::shared_ptr<Framebuffer> getShadowMapFramebuffer() const;

    private:
        void ensureFramebuffer(uint32_t size);
        glm::mat4 calculateLightSpaceMatrix(const DirectionalLight &light, float orthoSize = 20.0f) const;

    private:
        std::shared_ptr<Pipeline> m_shadowPipeline;
        std::shared_ptr<Pipeline> m_skinnedShadowPipeline;
        std::shared_ptr<Framebuffer> m_shadowMapFB;
        glm::mat4 m_lightSpaceMatrix{1.0f};
    };
} // namespace Fermion
