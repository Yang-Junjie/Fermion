#pragma once
#include "Fermion.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "Renderer/Renderers/SceneRenderer.hpp"

#include <functional>
#include <glm/glm.hpp>

namespace Fermion
{
    class ViewportPanel
    {
    public:
        struct ViewportBounds
        {
            glm::vec2 min{0.0f, 0.0f};
            glm::vec2 max{0.0f, 0.0f};

            [[nodiscard]] glm::vec2 size() const { return max - min; }
            [[nodiscard]] bool isValid() const
            {
                const glm::vec2 s = size();
                return s.x > 0.0f && s.y > 0.0f;
            }
            [[nodiscard]] bool contains(const glm::vec2 &p) const
            {
                return p.x >= min.x && p.y >= min.y &&
                       p.x < max.x && p.y < max.y;
            }
        };

        struct Context
        {
            std::shared_ptr<Framebuffer> framebuffer;
            std::shared_ptr<Scene> activeScene;
            std::shared_ptr<SceneRenderer> viewportRenderer;
            EditorCamera *editorCamera = nullptr;
            int sceneState = 0; // 0=Edit, 1=Play, 2=Simulate
            Entity selectedEntity;

            // Icon textures (non-owning)
            Texture2D *iconPlay = nullptr;
            Texture2D *iconStop = nullptr;
            Texture2D *iconPause = nullptr;
            Texture2D *iconSimulate = nullptr;
            Texture2D *iconStep = nullptr;
        };

        struct Callbacks
        {
            std::function<void()> onPlay;
            std::function<void()> onSimulate;
            std::function<void()> onStop;
            std::function<void(const std::filesystem::path &)> onOpenScene;
            std::function<void(const std::filesystem::path &)> onOpenProject;
            std::function<void(Entity)> onSelectEntity;
        };
        void setCallbacks(const Callbacks &callbacks) { m_callbacks = callbacks; }

        void onImGuiRender(const Context &ctx);
        void updateMousePicking(const Context &ctx);

        // Getters
        [[nodiscard]] const glm::vec2 &getViewportSize() const { return m_viewportSize; }
        [[nodiscard]] bool isViewportFocused() const { return m_viewportFocused; }
        [[nodiscard]] bool isViewportHovered() const { return m_viewportHovered; }
        [[nodiscard]] Entity getHoveredEntity() const { return m_hoveredEntity; }
        [[nodiscard]] int getGizmoType() const { return m_gizmoType; }
        [[nodiscard]] float getViewportTabBarHeight() const { return m_viewportTabBarHeight; }

        // Setters
        void setGizmoType(int type) { m_gizmoType = type; }
        void setHoveredEntity(Entity entity) { m_hoveredEntity = entity; }

    private:
        void onOverlayViewportUI(const Context &ctx);

        Callbacks m_callbacks;

        ViewportBounds m_viewport;
        glm::vec2 m_viewportSize{0.0f, 0.0f};
        bool m_viewportFocused = false;
        bool m_viewportHovered = false;
        Entity m_hoveredEntity;
        int m_gizmoType = -1;
        float m_viewportTabBarHeight = 0.0f;
    };
} // namespace Fermion
