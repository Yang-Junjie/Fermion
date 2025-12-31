#include "SceneRenderer.hpp"
#include "Renderer2D.hpp"
#include "Renderer3D.hpp"
#include "Renderer/RendererBackend.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Model/MeshFactory.hpp"
#include "Project/Project.hpp"

namespace Fermion {
    SceneRenderer::SceneRenderer() {
        m_debugRenderer = std::make_shared<DebugRenderer>();
        m_skybox = TextureCube::create("../Boson/projects/Assets/textures/skybox");
    }

    void SceneRenderer::beginScene(const Camera &camera, const glm::mat4 &transform) {
        beginScene({camera, glm::inverse(transform)});
    }

    void SceneRenderer::beginScene(const EditorCamera &camera) {
        beginScene({camera, camera.getViewMatrix()});
    }

    void SceneRenderer::beginScene(const SceneRendererCamera &camera) {
        m_sceneData.sceneCamera = camera;
        m_sceneData.sceneEnvironmentLight = m_scene->m_environmentLight;
        Renderer2D::beginScene(camera.camera, camera.view);
        Renderer3D::updateViewState(camera.camera, camera.view, m_sceneData.sceneEnvironmentLight);
    }

    void SceneRenderer::endScene() {
        FlushDrawList();
        Renderer2D::endScene();
    }

    void SceneRenderer::drawSprite(const glm::mat4 &transform, SpriteRendererComponent &sprite, int objectID) {
        if (static_cast<uint64_t>(sprite.textureHandle) != 0) {
            auto texture = Project::getRuntimeAssetManager()->getAsset<Texture2D>(sprite.textureHandle);
            Renderer2D::drawQuad(transform, texture,
                                 sprite.tilingFactor, sprite.color, objectID);
        } else {
            Renderer2D::drawQuad(transform, sprite.color, objectID);
        }
    }

    void SceneRenderer::drawString(const std::string &string, const glm::mat4 &transform,
                                   const TextComponent &component, int objectID) {
        Renderer2D::drawString(string, component.fontAsset, transform,
                               {component.color, component.kerning, component.lineSpacing}, objectID);
    }

    void SceneRenderer::drawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness, float fade,
                                   int objectID) {
        Renderer2D::drawCircle(transform, color, thickness, fade, objectID);
    }

    void SceneRenderer::drawRect(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color,
                                 int objectId) {
        Renderer2D::drawRect(position, size, color, objectId);
    }

    void SceneRenderer::drawRect(const glm::mat4 &transform, const glm::vec4 &color, int objectId) {
        Renderer2D::drawRect(transform, color, objectId);
    }

    void SceneRenderer::drawQuadBillboard(const glm::vec3 &translation, const glm::vec2 &size, const glm::vec4 &color,
                                          int objectId) {
        Renderer2D::drawQuadBillboard(translation, size, color, objectId);
    }
    void SceneRenderer::drawQuadBillboard(const glm::vec3 &translation, const glm::vec2 &size,
                                          const std::shared_ptr<Texture2D> &texture, float tilingFactor,
                                          const glm::vec4 &tintColor, int objectId) {
        Renderer2D::drawQuadBillboard(translation, size, texture, tilingFactor, tintColor, objectId);
    }

    void SceneRenderer::submitMesh(MeshComponent &meshComponent, glm::mat4 transform, int objectId, bool drawOutline) {
        if (static_cast<uint64_t>(meshComponent.meshHandle) != 0) {
            auto mesh = Project::getRuntimeAssetManager()->getAsset<Mesh>(meshComponent.meshHandle);
            if (mesh)
                s_MeshDrawList.push_back({mesh, nullptr, transform, objectId, drawOutline});
        }
    }

    void SceneRenderer::submitMesh(MeshComponent &meshComponent, MaterialComponent &materialComponent,
                                   glm::mat4 transform, int objectId, bool drawOutline) {
        if (static_cast<uint64_t>(meshComponent.meshHandle) != 0) {
            auto mesh = Project::getRuntimeAssetManager()->getAsset<Mesh>(meshComponent.meshHandle);
            if (mesh)
                s_MeshDrawList.push_back({mesh, materialComponent.MaterialInstance, transform, objectId, drawOutline});
        }
    }

    void SceneRenderer::drawLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &color) {
        Renderer2D::drawLine(start, end, color);
    }

    void SceneRenderer::setLineWidth(float thickness) {
        Renderer2D::setLineWidth(thickness);
    }

    SceneRenderer::Statistics SceneRenderer::getStatistics() const {
        Renderer2D::Satistics stats2D = Renderer2D::getStatistics();
        Statistics result;
        result.drawCalls = stats2D.drawCalls;
        result.quadCount = stats2D.quadCount;
        result.lineCount = stats2D.lineCount;
        result.circleCount = stats2D.circleCount;
        return result;
    }

    void SceneRenderer::GeometryPass() {
        m_RenderGraph.AddPass(
            {
                .Name = "GeometryPass",
                .Execute = [this](CommandBuffer &commandBuffer) {
                    Renderer3D::recordGeometryPass(commandBuffer, s_MeshDrawList);
                }
            });
    }

    void SceneRenderer::OutlinePass() {
        m_RenderGraph.AddPass(
            {
                .Name = "OutlinePass",
                .Execute = [this](CommandBuffer &commandBuffer) {
                    Renderer3D::recordOutlinePass(commandBuffer, s_MeshDrawList);
                }
            });
    }


    void SceneRenderer::SkyboxPass() {
        m_RenderGraph.AddPass(
            {
                .Name = "SkyboxPass",
                .Execute = [this](CommandBuffer &commandBuffer) {
                    Renderer3D::recordSkyboxPass(commandBuffer, m_skybox.get(), m_sceneData.sceneCamera.view,
                                                 m_sceneData.sceneCamera.camera.getProjection());
                }
            });
    }

    void SceneRenderer::FlushDrawList() {
        m_RenderGraph.Reset();
        if (m_sceneData.showSkybox)
            SkyboxPass();
        OutlinePass();

        GeometryPass();

        RendererBackend backend(RenderCommand::GetRendererAPI());
        m_RenderGraph.Execute(m_CommandQueue, backend);
        s_MeshDrawList.clear();
    }
} // namespace Fermion
