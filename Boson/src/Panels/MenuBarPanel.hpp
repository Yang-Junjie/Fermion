#pragma once

#include <functional>

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
        MenuBarPanel() = default;
        ~MenuBarPanel() = default;

        void SetCallbacks(const MenuBarCallbacks &callbacks) { m_Callbacks = callbacks; }
        void OnImGuiRender();
        float GetMenuBarHeight() const
        {
            return m_MenuBarHeight;
        }

    private:
        MenuBarCallbacks m_Callbacks;
        float m_MenuBarHeight = 35.0f;
    };
}
