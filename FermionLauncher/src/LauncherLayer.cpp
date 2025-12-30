#include "LauncherLayer.hpp"

#include <format>
#include <imgui.h>
#include "Utils/PlatformUtils.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <process.h>
#include <vector>

namespace
{
bool isProjectDescriptor(const std::filesystem::path &filePath) {
    auto ext = filePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return ext == ".fmproj";
}

std::filesystem::path findProjectFileInDirectory(const std::filesystem::path &directory) {
    std::error_code ec;
    for (std::filesystem::directory_iterator it(directory, ec), end; it != end; it.increment(ec)) {
        if (ec)
            break;

        std::error_code fileEc;
        if (!it->is_regular_file(fileEc) || fileEc)
            continue;

        if (isProjectDescriptor(it->path()))
            return it->path();
    }
    return {};
}

std::filesystem::path resolveBosonExecutable() {
    std::vector<std::filesystem::path> candidates;
    candidates.emplace_back("BosonEditor.exe");

    std::error_code ec;
    std::filesystem::path cwd = std::filesystem::current_path(ec);
    if (!ec) {
        candidates.emplace_back(cwd / "BosonEditor.exe");
        candidates.emplace_back(cwd / "bin" / "BosonEditor.exe");
        auto parent = cwd.parent_path();
        if (!parent.empty())
            candidates.emplace_back(parent / "bin" / "BosonEditor.exe");
    }

    for (auto &candidate : candidates) {
        std::error_code existsEc;
        if (candidate.empty())
            continue;

        if (std::filesystem::exists(candidate, existsEc) && !existsEc)
            return candidate;
    }

    return {};
}
} // namespace

LauncherLayer::LauncherLayer() : Fermion::Layer("NeutrinoLayer") {
    std::error_code ec;
    auto cwd = std::filesystem::current_path(ec);
    if (!ec) {
        m_projectsRoot = cwd;
    }
}

void LauncherLayer::onAttach() {
    scanProjects();
}

void LauncherLayer::onDetach() {
}

void LauncherLayer::onUpdate(Fermion::Timestep dt) {
}

void LauncherLayer::onEvent(Fermion::IEvent &event) {
}

bool LauncherLayer::onKeyPressedEvent(const Fermion::KeyPressedEvent &e) {
    return false;
}

void LauncherLayer::onImGuiRender() {
    static bool dockspaceOpen = true;
    if (dockspaceOpen) {
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen) {
            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImVec2 dockspacePos = viewport->WorkPos;
            ImVec2 dockspaceSize = viewport->WorkSize;
            ImGui::SetNextWindowPos(dockspacePos);
            ImGui::SetNextWindowSize(dockspaceSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        } else {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        // DockSpace
        {
            ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
            if (!opt_padding)
                ImGui::PopStyleVar();

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);

            ImGuiIO &io = ImGui::GetIO();
            ImGuiStyle &style = ImGui::GetStyle();
            float minWinSizeX = style.WindowMinSize.x;
            style.WindowMinSize.x = 370.0f;
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }
            style.WindowMinSize.x = minWinSizeX;
            ImGui::End();
        }
        // Scene Hierarchy

        drawProjectsListPanel();
        drawSettingPanel();
    }
}

void LauncherLayer::drawProjectsListPanel() {
    static int selectedIndex = -1;

    ImGui::SetNextWindowDockID(ImGui::GetID("MyDockSpace"), ImGuiCond_Once);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Projects List", nullptr, flags);

    // ===== 顶部标题 =====
    ImGuiIO& io = ImGui::GetIO();
    ImFont* largeFont = io.Fonts->Fonts.size() > 1 ? io.Fonts->Fonts[1] : nullptr;

    if (largeFont)
        ImGui::PushFont(largeFont);

    ImGui::Text("Projects:");

    if (largeFont)
        ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ===== 项目列表 =====
    if (m_projects.empty()) {
        ImGui::TextDisabled("No projects found.");
        ImGui::End();
        return;
    }

    const float cardHeight = 64.0f;
    const float padding = 8.0f;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (int i = 0; i < (int)m_projects.size(); i++) {
        ImGui::PushID(i);

        ImVec2 startPos = ImGui::GetCursorScreenPos();
        ImVec2 size(ImGui::GetContentRegionAvail().x, cardHeight);

        ImGui::InvisibleButton("ProjectCard", size);

        bool hovered  = ImGui::IsItemHovered();
        bool clicked  = ImGui::IsItemClicked();
        bool dblClick = hovered && ImGui::IsMouseDoubleClicked(0);

        if (clicked)
            selectedIndex = i;

        if (dblClick)
            openProject(m_projects[i]);

        ImU32 bgColor;
        if (selectedIndex == i)
            bgColor = IM_COL32(60, 120, 200, 180);
        else if (hovered)
            bgColor = IM_COL32(70, 70, 70, 180);
        else
            bgColor = IM_COL32(50, 50, 50, 140);

        drawList->AddRectFilled(
            startPos,
            { startPos.x + size.x, startPos.y + size.y },
            bgColor,
            6.0f
        );

        if (selectedIndex == i) {
            drawList->AddRect(
                startPos,
                { startPos.x + size.x, startPos.y + size.y },
                IM_COL32(100, 180, 255, 220),
                6.0f,
                0,
                2.0f
            );
        }

        ImVec2 textPos(startPos.x + padding, startPos.y + padding);

        drawList->AddText(
            textPos,
            IM_COL32(255, 255, 255, 255),
            m_projects[i].Name.c_str()
        );

        drawList->AddText(
            { textPos.x, textPos.y + 22 },
            IM_COL32(180, 180, 180, 200),
            m_projects[i].Path.string().c_str()
        );

        ImGui::Dummy({ 0.0f, 6.0f });
        ImGui::PopID();
    }

    ImGui::End();
}


void LauncherLayer::drawSettingPanel() {
    ImGuiWindowFlags flags =
    ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Launcher Settings", nullptr, flags);

    ImGui::Text("Projects Root Directory");

    constexpr size_t pathBufferSize = 512;
    static char pathBuffer[pathBufferSize]{};
    static std::string cachedPathValue;

    auto syncBufferWithRoot = [&](const std::string &value) {
        cachedPathValue = value;
        std::fill_n(pathBuffer, pathBufferSize, '\0');
        if (!value.empty()) {
            const size_t copyLength = std::min(value.size(), pathBufferSize - 1);
            std::copy_n(value.begin(), copyLength, pathBuffer);
        }
    };

    std::string currentRoot = m_projectsRoot.string();
    if (cachedPathValue != currentRoot) {
        syncBufferWithRoot(currentRoot);
    }

    if (ImGui::InputText("##ProjectRoot", pathBuffer, sizeof(pathBuffer))) {
        m_projectsRoot = std::filesystem::path(pathBuffer);
        syncBufferWithRoot(m_projectsRoot.string());
        scanProjects();
        currentRoot = m_projectsRoot.string();
    }

    if (ImGui::Button("Set Directory")) {
        auto selectedDirectory = Fermion::FileDialogs::selectDirectory(currentRoot);
        if (!selectedDirectory.empty()) {
            m_projectsRoot = selectedDirectory;
            syncBufferWithRoot(m_projectsRoot.string());
            scanProjects();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Rescan Projects")) {
        scanProjects();
    }
    ImGui::Separator();
    ImGui::TextDisabled("All projects are stored in one directory for now.");

    ImGui::End();
}

void LauncherLayer::scanProjects() {
    m_projects.clear();

    if (m_projectsRoot.empty())
        return;

    std::error_code ec;
    if (!std::filesystem::exists(m_projectsRoot, ec) || ec)
        return;

    if (!std::filesystem::is_directory(m_projectsRoot, ec) || ec)
        return;

    ec.clear();
    for (std::filesystem::directory_iterator it(m_projectsRoot, ec), end; it != end; it.increment(ec)) {
        if (ec)
            break;

        const auto &entryPath = it->path();

        std::error_code entryEc;
        if (it->is_directory(entryEc) && !entryEc) {
            auto projectFile = findProjectFileInDirectory(entryPath);
            if (projectFile.empty())
                continue;

            ProjectInfo info;
            info.ProjectFile = projectFile;
            info.Path = projectFile.parent_path();
            info.Name = projectFile.stem().string();
            m_projects.push_back(std::move(info));
            continue;
        }

        entryEc.clear();
        if (it->is_regular_file(entryEc) && !entryEc) {
            if (!isProjectDescriptor(entryPath))
                continue;

            ProjectInfo info;
            info.ProjectFile = entryPath;
            info.Path = entryPath.parent_path();
            info.Name = entryPath.stem().string();
            m_projects.push_back(std::move(info));
        }
    }

    std::sort(m_projects.begin(), m_projects.end(), [](const ProjectInfo &lhs, const ProjectInfo &rhs) {
        return lhs.Name < rhs.Name;
    });
}

void LauncherLayer::openProject(const ProjectInfo &project) {
    if (project.ProjectFile.empty()) {
        Fermion::Log::Warn(std::format("Selected project '{}' is missing a project file.", project.Name));
        return;
    }

    auto bosonExecutable = resolveBosonExecutable();
    if (bosonExecutable.empty()) {
        Fermion::Log::Error("Unable to find BosonEditor.exe. Please build the editor before opening projects.");
        return;
    }

    std::vector<std::string> arguments;
    arguments.emplace_back(project.ProjectFile.string());
    if (!Fermion::Process::launchDetached(bosonExecutable, arguments)) {
        Fermion::Log::Error(std::format("Failed to launch BosonEditor: {} {}", bosonExecutable.string(),
                                        project.ProjectFile.string()));
    }
}

