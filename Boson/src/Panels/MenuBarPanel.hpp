#pragma once
#include "fmpch.hpp"
#include "Renderer/Texture/Texture.hpp"
#include <imgui.h>

namespace Fermion {
    class BosonLayer;
    class MenuBarPanel {
    public:
        MenuBarPanel();

        ~MenuBarPanel() = default;

        void SetBosonLayer(BosonLayer *bosonLayer) {
            m_BosonLayer = bosonLayer;
        }

        void OnImGuiRender();

        float GetMenuBarHeight() const {
            return m_MenuBarHeight;
        }

    private:
        void DrawMenuItem(const char *label, const char *popupName, float &leftX, float y, float itemWidth,
                          float itemHeight) const;

        bool DrawWindowButton(const char *id,
                              const std::shared_ptr<Texture2D> &icon,
                              float &rightX,
                              float y,
                              float itemHeight,
                              ImU32 hoverColor = IM_COL32(0, 0, 0, 0)) const;

        void HandleWindowDrag(const ImVec2 &barPos, const ImVec2 &barSize);

        BosonLayer *m_BosonLayer = nullptr;
        float m_MenuBarHeight = 35.0f;

        std::unique_ptr<Texture2D> m_WindowMinimizeIcon, m_WindowMaximizeIcon, m_WindowRestoreIcon, m_WindowCloseIcon;
        bool m_IsWindowMaximized = false;

        inline static bool m_Dragging = false;
        inline static ImVec2 m_DragStartMouse;
        inline static ImVec2 m_DragStartWindow;
    };
} // namespace Fermion
