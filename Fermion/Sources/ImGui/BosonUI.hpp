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

        inline bool drawVec3Control(const std::string &label, glm::vec3 &values, float resetValue = 0.0f, float columnWidth = 100.0f, float dragSpeed = 0.1f)
        {
            bool changed = false;
            ImGui::PushID(label.c_str());

            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{0, 1});

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 1});

            if (ImGui::BeginTable("##Vec3Table", 2, ImGuiTableFlags_NoSavedSettings))
            {
                ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
                ImGui::TableSetupColumn("##controls", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetFontSize() + 2.0f);

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text(label.c_str());

                ImGui::TableNextColumn();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

                float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
                ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};
                float itemWidth = (ImGui::GetContentRegionAvail().x - buttonSize.x * 3.0f) / 3.0f;

                auto drawAxisControl = [&](const char *axisLabel, float &value, const ImVec4 &color, const ImVec4 &colorHovered)
                {
                    bool axisChanged = false;

                    ImGui::PushStyleColor(ImGuiCol_Button, color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorHovered);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

                    if (ImGui::Button(axisLabel, buttonSize))
                    {
                        value = resetValue;
                        axisChanged = true;
                    }
                    ImGui::PopStyleColor(3);

                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(itemWidth);

                    if (ImGui::DragFloat((std::string("##") + axisLabel).c_str(), &value, dragSpeed, 0.0f, 0.0f, "%.2f"))
                    {
                        axisChanged = true;
                    }
                    ImGui::SameLine();

                    return axisChanged;
                };

                changed |= drawAxisControl("X", values.x, {0.8f, 0.1f, 0.15f, 1.0f}, {0.9f, 0.2f, 0.2f, 1.0f});
                changed |= drawAxisControl("Y", values.y, {0.2f, 0.7f, 0.2f, 1.0f}, {0.3f, 0.8f, 0.3f, 1.0f});
                changed |= drawAxisControl("Z", values.z, {0.1f, 0.25f, 0.8f, 1.0f}, {0.2f, 0.35f, 0.9f, 1.0f});

                ImGui::PopStyleVar();
                ImGui::EndTable();
            }

            ImGui::PopStyleVar(2);
            ImGui::PopID();
            return changed;
        }
        inline bool drawVec2Control(const std::string &label,
                                    glm::vec2 &values,
                                    float resetValue = 0.0f,
                                    float columnWidth = 100.0f,
                                    float dragSpeed = 0.1f)
        {
            bool changed = false;
            ImGui::PushID(label.c_str());

            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{0, 1});
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 1});

            if (ImGui::BeginTable("##Vec2Table", 2, ImGuiTableFlags_NoSavedSettings))
            {
                ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
                ImGui::TableSetupColumn("##controls", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetFontSize() + 2.0f);

                // Label
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%s", label.c_str());

                // Controls
                ImGui::TableNextColumn();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

                float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
                ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};
                float itemWidth =
                    (ImGui::GetContentRegionAvail().x - buttonSize.x * 2.0f) / 2.0f;

                auto drawAxisControl = [&](const char *axisLabel,
                                           float &value,
                                           const ImVec4 &color,
                                           const ImVec4 &colorHovered)
                {
                    bool axisChanged = false;

                    ImGui::PushStyleColor(ImGuiCol_Button, color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorHovered);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

                    if (ImGui::Button(axisLabel, buttonSize))
                    {
                        value = resetValue;
                        axisChanged = true;
                    }
                    ImGui::PopStyleColor(3);

                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(itemWidth);

                    if (ImGui::DragFloat((std::string("##") + axisLabel).c_str(),
                                         &value,
                                         dragSpeed,
                                         0.0f,
                                         0.0f,
                                         "%.2f"))
                    {
                        axisChanged = true;
                    }

                    ImGui::SameLine();
                    return axisChanged;
                };

                changed |= drawAxisControl("X", values.x,
                                           {0.8f, 0.1f, 0.15f, 1.0f},
                                           {0.9f, 0.2f, 0.2f, 1.0f});

                changed |= drawAxisControl("Y", values.y,
                                           {0.2f, 0.7f, 0.2f, 1.0f},
                                           {0.3f, 0.8f, 0.3f, 1.0f});

                ImGui::PopStyleVar();
                ImGui::EndTable();
            }

            ImGui::PopStyleVar(2);
            ImGui::PopID();
            return changed;
        }
        inline bool drawFloatControl(const std::string &label,
                                     float &value,
                                     float columnWidth = 100.0f,
                                     float dragSpeed = 0.1f,
                                     float min = 0.0f,
                                     float max = 0.0f,
                                     const char *format = "%.2f")
        {
            bool changed = false;
            ImGui::PushID(label.c_str());

            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{0, 1});
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 1});

            if (ImGui::BeginTable("##FloatTable", 2, ImGuiTableFlags_NoSavedSettings))
            {
                ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
                ImGui::TableSetupColumn("##control", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetFontSize() + 2.0f);

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%s", label.c_str());

                ImGui::TableNextColumn();

                if (ImGui::DragFloat("##value", &value, dragSpeed, min, max, format))
                {
                    changed = true;
                }

                ImGui::EndTable();
            }

            ImGui::PopStyleVar(2);
            ImGui::PopID();
            return changed;
        }

        inline bool drawCheckboxControl(const std::string &label,
                                        bool &value,
                                        float columnWidth = 100.0f)
        {
            bool changed = false;
            ImGui::PushID(label.c_str());

            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{0, 1});
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 1});

            if (ImGui::BeginTable("##CheckboxTable", 2, ImGuiTableFlags_NoSavedSettings))
            {
                ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
                ImGui::TableSetupColumn("##control", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetFontSize() + 2.0f);

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%s", label.c_str());

                ImGui::TableNextColumn();

                if (ImGui::Checkbox("##value", &value))
                {
                    changed = true;
                }

                ImGui::EndTable();
            }

            ImGui::PopStyleVar(2);
            ImGui::PopID();
            return changed;
        }

        inline bool drawIntControl(const std::string &label,
                                   int &value,
                                   float columnWidth = 100.0f,
                                   int step = 1,
                                   int min = 0,
                                   int max = 0)
        {
            bool changed = false;
            ImGui::PushID(label.c_str());

            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{0, 1});
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 1});

            if (ImGui::BeginTable("##IntTable", 2, ImGuiTableFlags_NoSavedSettings))
            {
                ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
                ImGui::TableSetupColumn("##control", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetFontSize() + 2.0f);

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%s", label.c_str());

                ImGui::TableNextColumn();

                if (ImGui::DragInt("##value", &value, static_cast<float>(step), min, max))
                {
                    changed = true;
                }

                ImGui::EndTable();
            }

            ImGui::PopStyleVar(2);
            ImGui::PopID();
            return changed;
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
