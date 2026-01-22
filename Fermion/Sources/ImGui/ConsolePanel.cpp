#include "ConsolePanel.hpp"
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <cstdio>
namespace Fermion
{
    ConsolePanel::ConsolePanel()
    {
        memset(m_inputBuf, 0, sizeof(m_inputBuf));
        addLog("Console initialized.");
    }

    ConsolePanel::~ConsolePanel()
    {
        clear();
    }

    void ConsolePanel::clear()
    {
        for (int i = 0; i < m_items.Size; i++)
            free(m_items[i]);
        m_items.clear();
    }

    void ConsolePanel::addLog(const char *fmt, ...)
    {
        char buf[2048];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        buf[IM_ARRAYSIZE(buf) - 1] = 0;
        va_end(args);
#ifdef __linux__
            m_items.push_back(strdup(buf));
#else
        m_items.push_back(_strdup(buf));
#endif
        m_scrollToBottom = true;
    }

    void ConsolePanel::onImGuiRender()
    {
        ImGui::Begin("Console");

        // 输出区域
        {
            ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                              false, ImGuiWindowFlags_HorizontalScrollbar);

            for (int i = 0; i < m_items.Size; i++)
                ImGui::TextUnformatted(m_items[i]);

            if (m_scrollToBottom)
                ImGui::SetScrollHereY(1.0f);

            m_scrollToBottom = false;
            ImGui::EndChild();
        }

        ImGui::Separator();

        // 输入栏
        if (ImGui::InputText("Input", m_inputBuf, IM_ARRAYSIZE(m_inputBuf),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (m_inputBuf[0] != '\0')
            {
                addLog("> %s", m_inputBuf);

                // 命令解析
                if (strcmp(m_inputBuf, "clear") == 0)
                    clear();
                else if (strcmp(m_inputBuf, "help") == 0)
                    addLog("Commands: clear, help");

                m_inputBuf[0] = 0;
            }
        }

        ImGui::End();
    }
} // namespace Fermion
