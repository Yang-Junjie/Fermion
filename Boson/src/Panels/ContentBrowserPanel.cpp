#include "fmpch.hpp"
#include "ContentBrowserPanel.hpp"
#include "Asset/AssetExtensions.hpp"
#include <imgui.h>
#include <algorithm>
#include <cctype>

namespace {
    bool isProjectDescriptor(const std::filesystem::path &path) {
        auto ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return ext == ".fmproj";
    }
} // namespace

namespace Fermion {
    // 相对于可执行文件所在的 bin 目录
    constexpr const char *s_assetDirectory = "../Boson/projects";

    ContentBrowserPanel::ContentBrowserPanel() : m_currentDirectory(m_baseDirectory) {
        setBaseDirectory(std::filesystem::path(s_assetDirectory));
        m_directoryIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/DirectoryIcon.png");
        m_fileIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/FileIcon.png");
        m_meshFileIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/MeshFileIcon.png");
        m_textureFileIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/TextureFileIcon.png");
    }

    void ContentBrowserPanel::onImGuiRender() {
        ImGui::Begin("File Browser");
        drawFolderTree(m_baseDirectory);
        ImGui::End();

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

        for (auto &entry: std::filesystem::directory_iterator(m_currentDirectory)) {
            const auto &path = entry.path();
            if (entry.is_regular_file() && path.extension() == ".meta")
                continue;

            std::string filename = path.filename().string();
            ImGui::PushID(filename.c_str());

            AssetType extension = GetAssetTypeFromExtension(path.extension().string());
            bool isDirectory = entry.is_directory();

            Texture2D *icon;

            if (!isDirectory) {
                switch (extension) {
                    case AssetType::Mesh:
                        icon = m_meshFileIcon.get();
                        break;
                    case AssetType::Font:
                        icon = m_fileIcon.get();
                        break;
                    case AssetType::Texture:
                        icon = m_textureFileIcon.get();
                        break;
                    case AssetType::Shader:
                        icon = m_fileIcon.get();
                        break;
                    case AssetType::Scene:
                        icon = m_fileIcon.get();
                        break;
                    case AssetType::None:
                        icon = m_fileIcon.get();
                        break;
                    default:
                        icon = m_fileIcon.get();
                }
            } else {
                icon = m_directoryIcon.get();
            }
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

            // .fmscene 文件作为拖拽源
            if (!isDirectory && path.extension() == ".fmscene") {
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
            if (!isDirectory && (path.extension() == ".png" || path.extension() == ".jpg" || path.extension() ==
                                 ".jpeg")) {
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
                } else if (isProjectDescriptor(path) && m_projectOpenCallback) {
                    m_projectOpenCallback(path);
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
        std::filesystem::path normalized = directory;
        if (normalized.empty())
            normalized = std::filesystem::path(s_assetDirectory);

        if (normalized.empty())
            return;

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

    void ContentBrowserPanel::setProjectOpenCallback(std::function<void(const std::filesystem::path &)> callback) {
        m_projectOpenCallback = std::move(callback);
    }

    void ContentBrowserPanel::drawFolderTree(const std::filesystem::path &dir) {
        bool isSelected = (dir == m_currentDirectory);

        ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                ImGuiTreeNodeFlags_SpanAvailWidth;

        if (isSelected)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool opened = ImGui::TreeNodeEx(
            dir.string().c_str(),
            flags,
            "%s", dir.filename().string().c_str());

        if (ImGui::IsItemClicked()) {
            m_currentDirectory = dir;
        }

        if (opened) {
            for (auto &entry: std::filesystem::directory_iterator(dir)) {
                if (entry.is_directory()) {
                    drawFolderTree(entry.path());
                }
            }
            ImGui::TreePop();
        }
    }
} // namespace Fermion
