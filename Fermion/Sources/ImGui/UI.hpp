#pragma once
#include <imgui.h>
namespace Fermion
{
    namespace ui
    {
        inline bool BeginPopup(const char *str_id, ImGuiWindowFlags flags = 0)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f));

            bool is_open = ImGui::BeginPopup(str_id, flags);

            if (!is_open)
            {
                ImGui::PopStyleVar();
            }
            return is_open;
        }

        inline void EndPopup()
        {
            ImGui::EndPopup();
            ImGui::PopStyleVar();
        }

        inline void verticalProgressBar(float value, float minValue, float maxValue, ImVec2 size)
        {
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            ImU32 bg = IM_COL32(40, 40, 40, 255);
            ImU32 fill = IM_COL32(100, 220, 100, 255);
            ImU32 border = IM_COL32(200, 200, 200, 255);

            if (maxValue <= minValue)
                maxValue = minValue + 0.0001f;

            float fraction = (value - minValue) / (maxValue - minValue);

            if (fraction < 0.0f)
                fraction = 0.0f;
            if (fraction > 1.0f)
                fraction = 1.0f;

            ImVec2 maxPos(pos.x + size.x, pos.y + size.y);

            drawList->AddRectFilled(pos, maxPos, bg);
            drawList->AddRect(pos, maxPos, border);

            float h = size.y * fraction;

            if (h > 0.0f)
            {
                float topY = pos.y + size.y - h;

                float minY = pos.y + 2;
                float maxY = pos.y + size.y - 2;

                if (topY < minY)
                    topY = minY;
                if (topY > maxY)
                    topY = maxY;

                drawList->AddRectFilled(
                    ImVec2(pos.x + 2, topY),
                    ImVec2(pos.x + size.x - 2, maxY),
                    fill);
            }

            ImGui::Dummy(size);
        }
    }
}
