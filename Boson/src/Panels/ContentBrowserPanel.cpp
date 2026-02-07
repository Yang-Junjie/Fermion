#include "fmpch.hpp"
#include "ContentBrowserPanel.hpp"
#include "Asset/AssetExtensions.hpp"
#include <imgui.h>
#include <algorithm>
#include <cctype>
#include <vector>

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
    constexpr const char *s_assetDirectory = "../Boson/projects";

    ContentBrowserPanel::ContentBrowserPanel() : m_currentDirectory(m_baseDirectory)
    {
        setBaseDirectory(std::filesystem::path(s_assetDirectory));
        m_directoryIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/DirectoryIcon.png");
        m_fileIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/FileIcon.png");
        m_meshFileIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/MeshFileIcon.png");
        m_textureFileIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/TextureFileIcon.png");
        m_MaterialFileIcon = Texture2D::create("../Boson/Resources/Icons/ContentBrowser/MaterialFileIcon.png");

        m_materialThumbnailProvider = std::make_unique<MaterialThumbnailProvider>();
    }

    void ContentBrowserPanel::onImGuiRender()
    {
        ImGui::Begin("Content Browser");

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 4.0f));

        if (ImGui::BeginTable("ContentBrowserTable", 2,
                              ImGuiTableFlags_Resizable |
                                  ImGuiTableFlags_BordersInnerV |
                                  ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupColumn("FolderTree", ImGuiTableColumnFlags_WidthFixed, 250.0f);
            ImGui::TableSetupColumn("ContentView", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
            ImGui::BeginChild("FolderTree");

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
            drawFolderTree(m_baseDirectory);

            ImGui::EndChild();
            ImGui::PopStyleColor();

            ImGui::TableSetColumnIndex(1);
            ImGui::BeginChild("ContentView");

            if (!std::filesystem::exists(m_currentDirectory))
            {
                ImGui::Text("Directory not found: %s", m_currentDirectory.string().c_str());
                ImGui::EndChild();
                ImGui::EndTable();
                ImGui::PopStyleVar();
                ImGui::End();
                return;
            }

            // 顶部导航栏
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
                if (m_currentDirectory != m_baseDirectory)
                {
                    if (ImGui::Button(" <- Back "))
                        m_currentDirectory = m_currentDirectory.parent_path();
                }
                else
                {
                    ImGui::BeginDisabled();
                    ImGui::Button(" <- Back ");
                    ImGui::EndDisabled();
                }
                ImGui::SameLine();
                ImGui::Text("Current: %s", m_currentDirectory.generic_string().c_str());
                ImGui::PopStyleVar();
                ImGui::Separator();
            }

            static float padding = 16.0f;
            static float thumbnailSize = 80.0f;
            float cellSize = thumbnailSize + padding;

            float panelWidth = ImGui::GetContentRegionAvail().x;
            int columnCount = (int)(panelWidth / cellSize);
            if (columnCount < 1)
                columnCount = 1;

            if (ImGui::BeginTable("ContentGrid", columnCount))
            {
                std::vector<std::filesystem::directory_entry> entries;
                for (auto &entry : std::filesystem::directory_iterator(m_currentDirectory))
                    entries.push_back(entry);

                std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b)
                          {
                    if (a.is_directory() != b.is_directory()) return a.is_directory() > b.is_directory();
                    return a.path().filename().string() < b.path().filename().string(); });

                for (auto &entry : entries)
                {
                    const auto &path = entry.path();
                    if (entry.is_regular_file() && (path.extension() == ".meta"))
                        continue;

                    ImGui::TableNextColumn();
                    std::string filename = path.filename().string();
                    ImGui::PushID(filename.c_str());

                    AssetType extension = GetAssetTypeFromExtension(path.extension().string());
                    bool isDirectory = entry.is_directory();

                    const Texture2D *icon = nullptr;
                    const Texture2D *textureIcon = nullptr;
                    const Texture2D *materialIcon = nullptr;

                    if (extension == AssetType::TextureSource)
                        textureIcon = getOrCreateThumbnail(path, AssetType::TextureSource);

                    if (extension == AssetType::Material)
                        materialIcon = getOrCreateThumbnail(path, AssetType::Material);

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
                        case AssetType::TextureSource:
                            icon = textureIcon ? textureIcon : m_textureFileIcon.get();
                            break;
                        case AssetType::Shader:
                            icon = m_fileIcon.get();
                            break;
                        case AssetType::Scene:
                            icon = m_fileIcon.get();
                            break;
                        case AssetType::Material:
                            icon = materialIcon ? materialIcon : m_MaterialFileIcon.get();
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

                    // 绘制缩略图按钮
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0, 0, 0, 0});
                    bool clicked = false;
                    if (icon && icon->isLoaded())
                    {
                        clicked = ImGui::ImageButton("##icon", (ImTextureID)icon->getRendererID(),
                                                     ImVec2{thumbnailSize, thumbnailSize},
                                                     ImVec2{0, 1}, ImVec2{1, 0});
                    }
                    else
                    {
                        clicked = ImGui::Button("##icon", ImVec2{thumbnailSize, thumbnailSize});
                    }
                    ImGui::PopStyleColor();

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        if (isDirectory)
                            m_currentDirectory /= path.filename();
                        else if (isProjectDescriptor(path) && m_projectOpenCallback)
                            m_projectOpenCallback(path);
                    }

                    auto HandleDragDrop = [&](const char *type)
                    {
                        if (ImGui::BeginDragDropSource())
                        {
                            std::string fullPath = path.string();
                            ImGui::SetDragDropPayload(type, fullPath.c_str(), fullPath.size() + 1);

                            if (icon && icon->isLoaded())
                                ImGui::Image((ImTextureID)icon->getRendererID(), {32, 32}, {0, 1}, {1, 0});
                            ImGui::Text("%s", filename.c_str());
                            ImGui::EndDragDropSource();
                        }
                    };

                    if (!isDirectory)
                    {
                        if (path.extension() == ".fmscene")
                            HandleDragDrop("FERMION_SCENE");
                        else if (path.extension() == ".fmat")
                            HandleDragDrop("FERMION_MATERIAL");
                        else if (path.extension() == ".fmproj")
                            HandleDragDrop("FERMION_PROJECT");
                        else if (path.extension() == ".png" || path.extension() == ".jpg" || path.extension() == ".jpeg")
                            HandleDragDrop("FERMION_TEXTURE");
                        else if (path.extension() == ".hdr")
                            HandleDragDrop("FERMION_HDR");
                        else if (path.extension() == ".fmodel")
                            HandleDragDrop("FERMION_MODEL");
                        else if (path.extension() == ".fskel")
                            HandleDragDrop("FERMION_SKELETON");
                        else if (path.extension() == ".fanim")
                            HandleDragDrop("FERMION_ANIMATION");
                    }

                    ImGui::TextWrapped("%s", filename.c_str());

                    ImGui::PopID();
                }

                ImGui::EndTable(); // ContentGrid
            }

            ImGui::EndChild(); // ContentView
            ImGui::EndTable(); // ContentBrowserTable
        }
        ImGui::PopStyleVar(); // CellPadding

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

    void ContentBrowserPanel::setCurrentDirectory(const std::filesystem::path &directory)
    {
        m_currentDirectory = directory;
    }

    void ContentBrowserPanel::setProjectOpenCallback(std::function<void(const std::filesystem::path &)> callback)
    {
        m_projectOpenCallback = std::move(callback);
    }

    void ContentBrowserPanel::drawFolderTree(const std::filesystem::path &dir)
    {
        std::vector<std::filesystem::path> subdirs;
        try
        {
            for (auto &entry : std::filesystem::directory_iterator(dir))
            {
                if (entry.is_directory())
                {
                    subdirs.push_back(entry.path());
                }
            }
        }
        catch (...)
        {
        }

        std::sort(subdirs.begin(), subdirs.end());

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                   ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                   ImGuiTreeNodeFlags_SpanAvailWidth |
                                   ImGuiTreeNodeFlags_FramePadding;

        if (dir == m_currentDirectory)
            flags |= ImGuiTreeNodeFlags_Selected;

        if (subdirs.empty())
            flags |= ImGuiTreeNodeFlags_Leaf;

        bool opened = ImGui::TreeNodeEx(dir.string().c_str(), flags, "");

        if (ImGui::IsItemClicked())
        {
            m_currentDirectory = dir;
        }

        ImGui::SameLine();
        float lineHeight = ImGui::GetTextLineHeight();

        if (m_directoryIcon && m_directoryIcon->isLoaded())
        {
            float verticalOffset = (ImGui::GetFrameHeight() - lineHeight) * 0.5f;
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + verticalOffset);

            ImGui::Image((ImTextureID)m_directoryIcon->getRendererID(), {lineHeight, lineHeight}, {0, 1}, {1, 0});
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - verticalOffset);
            ImGui::SameLine();
        }

        ImGui::TextUnformatted(dir.filename().string().c_str());

        if (opened)
        {
            for (const auto &subdir : subdirs)
            {
                drawFolderTree(subdir);
            }
            ImGui::TreePop();
        }
    }

    const Texture2D *ContentBrowserPanel::getOrCreateThumbnail(const std::filesystem::path &path, AssetType type)
    {
        auto key = path.string();

        auto it = m_thumbnailCache.find(key);
        if (it != m_thumbnailCache.end())
        {
            return it->second.get();
        }

        std::unique_ptr<Texture2D> thumbnail = nullptr;

        switch (type)
        {
        case AssetType::TextureSource:
            thumbnail = Texture2D::create(key);
            break;

        case AssetType::Material:
            if (m_materialThumbnailProvider)
            {
                thumbnail = m_materialThumbnailProvider->generateThumbnail(path);
            }
            break;

        default:
            return nullptr;
        }

        if (thumbnail && thumbnail->isLoaded())
        {
            auto *ptr = thumbnail.get();
            m_thumbnailCache[key] = std::move(thumbnail);
            return ptr;
        }

        return nullptr;
    }

} // namespace Fermion