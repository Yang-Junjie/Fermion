#pragma once
#include <imgui.h>
#include <vector>
#include <string>

namespace Fermion
{
    class ConsolePanel
    {
    public:
        ConsolePanel();
        ~ConsolePanel();

        void addLog(const char *fmt, ...);
        void clear();
        void onImGuiRender();
        static ConsolePanel &get()
        {
            static ConsolePanel instance;
            return instance;
        }

    private:
        ImVector<char *> m_items;
        char m_inputBuf[256];
        bool m_scrollToBottom = false;
    };
}
