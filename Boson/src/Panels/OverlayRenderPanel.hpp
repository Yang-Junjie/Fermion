#pragma once
#include "Fermion.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "Renderer/Renderers/SceneRenderer.hpp"

namespace Fermion
{
    class OverlayRenderPanel
    {
    public:
        struct Context
        {
            std::shared_ptr<Scene> activeScene;
            std::shared_ptr<SceneRenderer> viewportRenderer;
            const EditorCamera *editorCamera = nullptr;
            int sceneState = 0; // 0=Edit, 1=Play, 2=Simulate
            bool showPhysicsColliders = false;
            Entity selectedEntity;
        };

        void render(const Context &ctx) const;

    private:
        bool beginOverlayPass(const Context &ctx) const;
        void renderPhysicsColliders(const Context &ctx) const;
        void renderPhysics2DColliders(const Context &ctx) const;
        void renderPhysics3DColliders(const Context &ctx) const;
        void renderSelectedEntityOutline(const Context &ctx) const;
    };
} // namespace Fermion
