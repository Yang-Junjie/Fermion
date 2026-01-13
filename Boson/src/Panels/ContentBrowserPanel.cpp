#include "fmpch.hpp"
#include "ContentBrowserPanel.hpp"
#include "Asset/AssetExtensions.hpp"
#include <imgui.h>
#include <algorithm>
#include <cctype>

namespace
{
    bool isProjectDescriptor(const std::filesystem::path &path)
    {
        auto ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c)
                       { return static_cast<char>(std::tolower(c)); });
        return ext == ".fmproj";
    }
} // namespace

namespace Fermion
{
    // 相对于可执行文件所在的 bin 目录
    constexpr const char *s_assetDirectory = "../Boson/projects";

    ContentBrowserPanel::ContentBrowserPanel() : m_currentDirectory(m_baseDirectory)
    {
        setBaseDirectory(std::filesystem::path(s_assetDirectory));
        m_directoryIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/DirectoryIcon.png");
        m_fileIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/FileIcon.png");
        m_meshFileIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/MeshFileIcon.png");
        m_textureFileIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/TextureFileIcon.png");
    }

    void ContentBrowserPanel::onImGuiRender()
    {
        ImGui::Begin("Content Browser");

        if (ImGui::BeginTable("ContentBrowserTable", 2,
                              ImGuiTableFlags_Resizable |
                                  ImGuiTableFlags_BordersInnerV |
                                  ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupColumn("FolderTree", ImGuiTableColumnFlags_WidthFixed, 300.0f);
            ImGui::TableSetupColumn("ContentView", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();

            // ================= 左侧文件树 =================
            ImGui::TableSetColumnIndex(0);
            ImGui::BeginChild("FolderTree");
            drawFolderTree(m_baseDirectory);
            ImGui::EndChild();

            // ================= 右侧内容视图 =================
            ImGui::TableSetColumnIndex(1);
            ImGui::BeginChild("ContentView");

            if (!std::filesystem::exists(m_currentDirectory))
            {
                ImGui::Text("Directory not found: %s", m_currentDirectory.string().c_str());
                ImGui::EndChild();
                ImGui::EndTable();
                ImGui::End();
                return;
            }

            if (m_currentDirectory != m_baseDirectory)
            {
                if (ImGui::Button("<-"))
                    m_currentDirectory = m_currentDirectory.parent_path();
            }

            static float padding = 16.0f;
            static float thumbnailSize = 80.0f;
            float cellSize = thumbnailSize + padding;

            float panelWidth = ImGui::GetContentRegionAvail().x;
            int columnCount = (int)(panelWidth / cellSize);
            if (columnCount < 1)
                columnCount = 1;

            // 网格 Table
            if (ImGui::BeginTable("ContentGrid", columnCount))
            {
                for (auto &entry : std::filesystem::directory_iterator(m_currentDirectory))
                {
                    const auto &path = entry.path();
                    if (entry.is_regular_file() &&( path.extension() == ".meta"))
                        continue;

                    ImGui::TableNextColumn();
                    std::string filename = path.filename().string();
                    ImGui::PushID(filename.c_str());

                    AssetType extension = GetAssetTypeFromExtension(path.extension().string());
                    bool isDirectory = entry.is_directory();

                    const Texture2D *icon = nullptr;
                    const Texture2D *textureIcon = nullptr;

                    if (extension == AssetType::Texture)
                        textureIcon = getOrCreateThumbnail(path);

                    if (!isDirectory)
                    {
                        switch (extension)
                        {
                        case AssetType::Mesh:
                            icon = m_meshFileIcon.get();
                            break;
                        case AssetType::Font:
                            icon = m_fileIcon.get();
                            break;
                        case AssetType::Texture:
                            icon = textureIcon ? textureIcon : m_textureFileIcon.get();
                            break;
                        case AssetType::Shader:
                            icon = m_fileIcon.get();
                            break;
                        case AssetType::Scene:
                            icon = m_fileIcon.get();
                            break;
                        default:
                            icon = m_fileIcon.get();
                            break;
                        }
                    }
                    else
                    {
                        icon = m_directoryIcon.get();
                    }

                    if (icon && icon->isLoaded())
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0, 0, 0, 0});
                        ImGui::ImageButton(
                            "##icon",
                            (ImTextureID)icon->getRendererID(),
                            ImVec2{thumbnailSize, thumbnailSize},
                            ImVec2{0, 1}, ImVec2{1, 0});
                        ImGui::PopStyleColor();
                    }
                    else
                    {
                        ImGui::Button("##icon", ImVec2{thumbnailSize, thumbnailSize});
                    }

                    if (!isDirectory && path.extension() == ".fmscene")
                    {
                        if (ImGui::BeginDragDropSource())
                        {
                            std::string fullPath = path.string();
                            ImGui::SetDragDropPayload("FERMION_SCENE", fullPath.c_str(), fullPath.size() + 1);
                            ImGui::Text("%s", filename.c_str());
                            ImGui::EndDragDropSource();
                        }
                    }
                    if (!isDirectory && path.extension() == ".fmat")
                    {
                        if (ImGui::BeginDragDropSource())
                        {
                            std::string fullPath = path.string();
                            ImGui::SetDragDropPayload("FERMION_MATERIAL", fullPath.c_str(), fullPath.size() + 1);
                            ImGui::Text("%s", filename.c_str());
                            ImGui::EndDragDropSource();
                        }
                    }

                    if (!isDirectory && path.extension() == ".fmproj")
                    {
                        if (ImGui::BeginDragDropSource())
                        {
                            std::string fullPath = path.string();
                            ImGui::SetDragDropPayload("FERMION_PROJECT", fullPath.c_str(), fullPath.size() + 1);
                            ImGui::Text("%s", filename.c_str());
                            ImGui::EndDragDropSource();
                        }
                    }

                    if (!isDirectory &&
                        (path.extension() == ".png" || path.extension() == ".jpg" || path.extension() == ".jpeg"))
                    {
                        if (ImGui::BeginDragDropSource())
                        {
                            std::string fullPath = path.string();
                            ImGui::SetDragDropPayload("FERMION_TEXTURE", fullPath.c_str(), fullPath.size() + 1);
                            ImGui::Text("%s", filename.c_str());
                            ImGui::EndDragDropSource();
                        }
                    }

                    if (!isDirectory && path.extension() == ".fmodel")
                    {
                        if (ImGui::BeginDragDropSource())
                        {
                            std::string fullPath = path.string();
                            ImGui::SetDragDropPayload("FERMION_MODEL", fullPath.c_str(), fullPath.size() + 1);
                            ImGui::Text("%s", filename.c_str());
                            ImGui::EndDragDropSource();
                        }
                    }

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        if (isDirectory)
                            m_currentDirectory /= path.filename();
                        else if (isProjectDescriptor(path) && m_projectOpenCallback)
                            m_projectOpenCallback(path);
                    }

                    ImGui::TextWrapped("%s", filename.c_str());

                    ImGui::PopID();
                }

                ImGui::EndTable(); // ContentGrid
            }

            ImGui::EndChild(); // ContentView
            ImGui::EndTable(); // ContentBrowserTable
        }

        ImGui::End();
    }

    void ContentBrowserPanel::setBaseDirectory(const std::filesystem::path &directory)
    {
        std::filesystem::path normalized = directory;
        if (normalized.empty())
            normalized = std::filesystem::path(s_assetDirectory);

        if (normalized.empty())
            return;

        if (!std::filesystem::exists(normalized))
        {
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

    void ContentBrowserPanel::setProjectOpenCallback(std::function<void(const std::filesystem::path &)> callback)
    {
        m_projectOpenCallback = std::move(callback);
    }

    void ContentBrowserPanel::drawFolderTree(const std::filesystem::path &dir)
    {
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

        if (ImGui::IsItemClicked())
        {
            m_currentDirectory = dir;
        }

        if (opened)
        {
            for (auto &entry : std::filesystem::directory_iterator(dir))
            {
                if (entry.is_directory())
                {
                    drawFolderTree(entry.path());
                }
            }
            ImGui::TreePop();
        }
    }
    const Texture2D *ContentBrowserPanel::getOrCreateThumbnail(const std::filesystem::path &path)
    {
        auto key = path.string();

        auto &slot = m_thumbnailCache[key];
        if (!slot)
            slot = Texture2D::create(key);

        return slot.get();
    }

} // namespace Fermion
