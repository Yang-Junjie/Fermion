#include "fmpch.hpp"
#include "ContentBrowserPanel.hpp"

#include <imgui.h>

namespace Fermion {
// 相对于可执行文件所在的 bin 目录
constexpr const char *s_assetDirectory = "../Boson/projects";

ContentBrowserPanel::ContentBrowserPanel() : m_baseDirectory(std::filesystem::path(s_assetDirectory)),
                                             m_currentDirectory(m_baseDirectory) {
    m_directoryIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/DirectoryIcon.png");
    m_fileIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/FileIcon.png");
}

void ContentBrowserPanel::onImGuiRender() {
    ImGui::Begin("Content Browser");

    if (!std::filesystem::exists(m_currentDirectory)) {
        ImGui::Text("Directory not found: %s", m_currentDirectory.string().c_str());
        ImGui::End();
        return;
    }

    if (m_currentDirectory != m_baseDirectory) {
        if (ImGui::Button("<-")) {
            m_currentDirectory = m_currentDirectory.parent_path();
        }
    }

    static float padding = 16.0f;
    static float thumbnailSize = 80.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = static_cast<int>(panelWidth / cellSize);
    if (columnCount < 1) {
        columnCount = 1;
    }

    ImGui::Columns(columnCount, nullptr, false);

    for (auto &entry : std::filesystem::directory_iterator(m_currentDirectory)) {
        const auto &path = entry.path();
        std::string filename = path.filename().string();
        bool isDirectory = entry.is_directory();
        if (entry.is_regular_file() && path.extension() == ".meta")
            continue;
        ImGui::PushID(filename.c_str());

        std::shared_ptr<Texture2D> icon = isDirectory ? m_directoryIcon : m_fileIcon;
        if (icon && icon->isLoaded()) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0, 0, 0, 0});
            ImGui::ImageButton(
                filename.c_str(),
                ImTextureRef(static_cast<ImTextureID>(icon->getRendererID())),
                ImVec2{thumbnailSize, thumbnailSize},
                ImVec2{0, 1}, ImVec2{1, 0});
            ImGui::PopStyleColor();
        } else {
            ImGui::Button(filename.c_str(), ImVec2{thumbnailSize, thumbnailSize});
        }

        // .fermion 文件作为拖拽源
        if (!isDirectory && path.extension() == ".fermion") {
            if (ImGui::BeginDragDropSource()) {
                std::string fullPath = path.string();
                ImGui::SetDragDropPayload("FERMION_SCENE", fullPath.c_str(), fullPath.size() + 1);
                ImGui::Text("%s", filename.c_str());
                ImGui::EndDragDropSource();
            }
        }
        // .fmproj 文件作为拖拽源
        if (!isDirectory && path.extension() == ".fmproj") {
            if (ImGui::BeginDragDropSource()) {
                std::string fullPath = path.string();
                ImGui::SetDragDropPayload("FERMION_PROJECT", fullPath.c_str(), fullPath.size() + 1);
                ImGui::Text("%s", filename.c_str());
                ImGui::EndDragDropSource();
            }
        }
        //.png or 图片
        if (!isDirectory && (path.extension() == ".png" || path.extension() == ".jpg" || path.extension() == ".jpeg")) {
            if (ImGui::BeginDragDropSource()) {
                std::string fullPath = path.string();
                ImGui::SetDragDropPayload("FERMION_TEXTURE", fullPath.c_str(), fullPath.size() + 1);
                ImGui::Text("%s", filename.c_str());
                ImGui::EndDragDropSource();
            }
        }

        // .obj
        if (!isDirectory && (path.extension() == ".obj")) {
            if (ImGui::BeginDragDropSource()) {
                std::string fullPath = path.string();
                ImGui::SetDragDropPayload("FERMION_MODEL", fullPath.c_str(), fullPath.size() + 1);
                ImGui::Text("%s", filename.c_str());
                ImGui::EndDragDropSource();
            }
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (isDirectory) {
                m_currentDirectory /= path.filename();
            }
        }

        ImGui::TextWrapped("%s", filename.c_str());

        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::Columns(1);

    ImGui::End();
}

void ContentBrowserPanel::setBaseDirectory(const std::filesystem::path &directory) {
    if (directory.empty())
        return;

    std::filesystem::path normalized = directory;
    if (!std::filesystem::exists(normalized)) {
        std::filesystem::create_directories(normalized);
        if (!std::filesystem::exists(normalized))
            return;
    }

    if (!std::filesystem::is_directory(normalized))
        normalized = normalized.parent_path();

    if (normalized.empty())
        return;

    m_baseDirectory = normalized;
    m_currentDirectory = m_baseDirectory;
}
} // namespace Fermion
