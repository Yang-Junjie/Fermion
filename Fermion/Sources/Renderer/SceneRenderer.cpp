#include "SceneRenderer.hpp"
#include "Renderer2D.hpp"
#include "Renderer3D.hpp"
#include "Project/Project.hpp"
namespace Fermion
{
    SceneRenderer::SceneRenderer()
    {
        m_debugRenderer = std::make_shared<DebugRenderer>();
    }

    void SceneRenderer::beginScene(const Camera &camera, const glm::mat4 &transform)
    {
        beginScene({camera, glm::inverse(transform)});
    }

    void SceneRenderer::beginScene(const EditorCamera &camera)
    {

        beginScene({camera, camera.getViewMatrix()});
    }

    void SceneRenderer::beginScene(const SceneRendererCamera &camera)
    {
        m_sceneData.sceneCamera = camera;
        Renderer2D::beginScene(camera.camera, camera.view);
        Renderer3D::SetCamera(camera.camera, camera.view);
    }

    void SceneRenderer::endScene()
    {
        Renderer2D::endScene();

        m_RenderGraph.Reset();
        GeometryPass();
        OutlinePass();
        m_RenderGraph.Execute();
        s_MeshDrawList.clear();
    }

    void SceneRenderer::drawSprite(const glm::mat4 &transform, SpriteRendererComponent &sprite, int objectID)
    {
        if (static_cast<uint64_t>(sprite.textureHandle) != 0)
        {
            auto texture = Project::getRuntimeAssetManager()->getAsset<Texture2D>(sprite.textureHandle);
            Renderer2D::drawQuad(transform, texture,
                                 sprite.tilingFactor, sprite.color, objectID);
        }
        else
        {

            Renderer2D::drawQuad(transform, sprite.color, objectID);
        }
    }

    void SceneRenderer::drawString(const std::string &string, const glm::mat4 &transform, const TextComponent &component, int objectID)
    {
        Renderer2D::drawString(string, component.fontAsset, transform, {component.color, component.kerning, component.lineSpacing}, objectID);
    }

    void SceneRenderer::drawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness, float fade, int objectID)
    {
        Renderer2D::drawCircle(transform, color, thickness, fade, objectID);
    }
    void SceneRenderer::drawRect(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color, int objectId)
    {
        Renderer2D::drawRect(position, size, color, objectId);
    }
    void SceneRenderer::drawRect(const glm::mat4 &transform, const glm::vec4 &color, int objectId)
    {
        Renderer2D::drawRect(transform, color, objectId);
    }

   

    void SceneRenderer::SubmitMesh(MeshComponent &meshComponent, glm::mat4 transform, int objectId, bool drawOutline)
    {
        // if (static_cast<uint64_t>(meshComponent.meshHandle) != 0)
        // {
        //     // Log::Error(std::format("{}",std::to_string(meshComponent.meshHandle)));
        //     auto mesh = Project::getRuntimeAssetManager()->getAsset<Mesh>(meshComponent.meshHandle);
        //     Renderer3D::DrawMesh(mesh, transform, objectId);
        //     if (drawOutline)
        //         Renderer3D::DrawMeshOutline(mesh, transform, objectId);
        // }
        auto mesh = Project::getRuntimeAssetManager()->getAsset<Mesh>(meshComponent.meshHandle);
        s_MeshDrawList.push_back({mesh, nullptr, transform, objectId, drawOutline});
    }

    void SceneRenderer::SubmitMesh(MeshComponent &meshComponent, MaterialComponent &materialComponent, glm::mat4 transform, int objectId, bool drawOutline)
    {
        // if (static_cast<uint64_t>(meshComponent.meshHandle) != 0)
        // {
        //     auto mesh = Project::getRuntimeAssetManager()->getAsset<Mesh>(meshComponent.meshHandle);
        //     if (materialComponent.overrideMaterial)
        //     {
        //         Renderer3D::DrawMesh(mesh, materialComponent.MaterialInstance, transform, objectId);
        //     }
        //     else
        //     {
        //         Renderer3D::DrawMesh(mesh, transform, objectId);
        //     }
        //     if (drawOutline)
        //         Renderer3D::DrawMeshOutline(mesh, transform, objectId);
        // }
        auto mesh = Project::getRuntimeAssetManager()->getAsset<Mesh>(meshComponent.meshHandle);
        s_MeshDrawList.push_back({mesh, materialComponent.MaterialInstance, transform, objectId, drawOutline});
    }

    void SceneRenderer::DrawLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &color)
    {
        Renderer2D::drawLine(start, end, color);
    }

    void SceneRenderer::SetLineWidth(float thickness)
    {
        Renderer2D::setLineWidth(thickness);
    }

    SceneRenderer::Statistics SceneRenderer::getStatistics() const
    {
        Renderer2D::Satistics stats2D = Renderer2D::getStatistics();
        Statistics result;
        result.drawCalls = stats2D.drawCalls;
        result.quadCount = stats2D.quadCount;
        result.lineCount = stats2D.lineCount;
        result.circleCount = stats2D.circleCount;
        return result;
    }
    void SceneRenderer::GeometryPass()
    {
        m_RenderGraph.AddPass({ .Name = "GeometryPass",.Execute = [this]()
        {
            for (auto &mesh : s_MeshDrawList)
            {
                if (mesh.material)
                {
                    Renderer3D::DrawMesh(mesh.mesh, mesh.material, mesh.transform, mesh.objectID);
                }
                else
                {
                    Renderer3D::DrawMesh(mesh.mesh, mesh.transform, mesh.objectID);
                }
            }
        }});
    }
    void SceneRenderer::OutlinePass()
    {
        m_RenderGraph.AddPass({ .Name = "OutlinePass",.Execute = [this]()
        {
            for (auto &mesh : s_MeshDrawList)
            {
                if (mesh.drawOutline)
                {
                    Renderer3D::DrawMeshOutline(mesh.mesh, mesh.transform, mesh.objectID);
                }
            }
        }});
    }
}
