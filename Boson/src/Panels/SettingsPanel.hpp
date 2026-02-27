#pragma once
#include "Fermion.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "Renderer/Renderers/SceneRenderer.hpp"
#include "Renderer/Font/Font.hpp"

namespace Fermion
{
    class SettingsPanel
    {
    public:
        struct Context
        {
            std::shared_ptr<SceneRenderer> viewportRenderer;
            EditorCamera *editorCamera = nullptr;
            Entity hoveredEntity;
            bool viewportFocused = false;
            bool *showPhysicsDebug = nullptr;
            bool *showRenderEntities = nullptr;
        };

        void onImGuiRender(const Context &ctx);

    private:
        void renderRendererInfo(const Context &ctx);
        void renderEnvironmentSettings(const Context &ctx);
        void renderDebugSettings(const Context &ctx);
        void renderProjectSettings(const Context &ctx);
    };
} // namespace Fermion
