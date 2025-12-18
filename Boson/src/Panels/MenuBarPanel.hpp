#pragma once
#include "fmpch.hpp"
#include "Renderer/Texture.hpp"
#include <functional>
#include <imgui.h>
namespace Fermion
{
    // TODO(Yang):迁移函数从BosonLayer to MenuBarPanel
    struct MenuBarCallbacks
    {
        std::function<void()> NewScene;
        std::function<void()> OpenScene;
        std::function<void()> SaveScene;
        std::function<void()> SaveSceneAs;
        std::function<void()> NewProject;
        std::function<void()> OpenProject;
        std::function<void()> SaveProject;
        std::function<void()> ExitApplication;
        std::function<void()> ShowAbout;
    };

    class MenuBarPanel
    {
    public:
        MenuBarPanel();
        ~MenuBarPanel() = default;

        void SetCallbacks(const MenuBarCallbacks &callbacks) { m_Callbacks = callbacks; }
        void OnImGuiRender();
        float GetMenuBarHeight() const
        {
            return m_MenuBarHeight;
        }

        void InvokeCallback(const std::function<void()> &fn) const;

    private:
        void DrawMenuItem(const char *label, const char *popupName, float &leftX, float y, float itemWidth, float itemHeight) const;
        bool DrawWindowButton(const char *id,
                              const std::shared_ptr<Texture2D> &icon,
                              float &rightX,
                              float y,
                              float itemHeight,
                              ImU32 hoverColor = IM_COL32(0, 0, 0, 0)) const;
        void HandleWindowDrag(const ImVec2 &barPos, const ImVec2 &barSize);

        MenuBarCallbacks m_Callbacks;
        float m_MenuBarHeight = 35.0f;

        std::shared_ptr<Texture2D> m_WindowMinimizeIcon, m_WindowMaximizeIcon, m_WindowRestoreIcon, m_WindowCloseIcon;
        bool m_IsWindowMaximized = false;

        inline static bool m_Dragging = false;
        inline static ImVec2 m_DragStartMouse;
        inline static ImVec2 m_DragStartWindow;
    };
}
