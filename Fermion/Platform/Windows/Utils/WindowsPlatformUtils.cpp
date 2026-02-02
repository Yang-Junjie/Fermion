#include "fmpch.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Core/Application.hpp"
#include "Core/Window.hpp"
#include <windows.h>
#include <shobjidl.h>
#include <process.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Fermion
{

    float Time::getTime()
    {
        return glfwGetTime();
    }

    std::filesystem::path FileDialogs::openFile(const char *filter, std::string defaultPath)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window(
            (GLFWwindow *)Application::get().getWindow().getNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrInitialDir = defaultPath.c_str();
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE)
            return std::filesystem::path(ofn.lpstrFile);

        return {};
    }

    std::filesystem::path FileDialogs::saveFile(const char *filter, std::string defaultPath)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window(
            (GLFWwindow *)Application::get().getWindow().getNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrInitialDir = defaultPath.c_str();
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        // Set default extension (comes from filter)
        ofn.lpstrDefExt = strchr(filter, '\0') + 1;

        if (GetSaveFileNameA(&ofn) == TRUE)
            return std::filesystem::path(ofn.lpstrFile);

        return {};
    }
    std::filesystem::path FileDialogs::selectDirectory(std::string defaultPath)
    {
        // 初始化COM
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (FAILED(hr))
            return {};

        std::filesystem::path resultPath;

        IFileDialog *pFileDialog = nullptr;
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
        if (SUCCEEDED(hr))
        {
            // 设置选择文件夹模式
            DWORD options;
            pFileDialog->GetOptions(&options);
            pFileDialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

            // 设置初始目录
            if (!defaultPath.empty())
            {
                std::filesystem::path defaultPathW(defaultPath);
                IShellItem *psiFolder = nullptr;
                if (SUCCEEDED(SHCreateItemFromParsingName(defaultPathW.wstring().c_str(), nullptr, IID_PPV_ARGS(&psiFolder))))
                {
                    pFileDialog->SetFolder(psiFolder);
                    psiFolder->Release();
                }
            }

            // 显示对话框
            if (SUCCEEDED(pFileDialog->Show(glfwGetWin32Window(
                    (GLFWwindow *)Application::get().getWindow().getNativeWindow()))))
            {
                IShellItem *psiResult = nullptr;
                if (SUCCEEDED(pFileDialog->GetResult(&psiResult)))
                {
                    PWSTR pszPath = nullptr;
                    if (SUCCEEDED(psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)))
                    {
                        resultPath = pszPath;
                        CoTaskMemFree(pszPath);
                    }
                    psiResult->Release();
                }
            }

            pFileDialog->Release();
        }

        CoUninitialize();
        return resultPath;
    }

    bool Process::launchDetached(const std::filesystem::path &executablePath, const std::vector<std::string> &arguments)
    {
        if (executablePath.empty())
            return false;

        std::wstring exe = executablePath.wstring();
        std::vector<std::wstring> wideArgs;
        wideArgs.reserve(arguments.size() + 1);
        wideArgs.push_back(exe);
        for (const auto &arg : arguments)
        {
            std::filesystem::path argPath(arg);
            wideArgs.emplace_back(argPath.wstring());
        }

        std::vector<const wchar_t *> argv;
        argv.reserve(wideArgs.size() + 1);
        for (const auto &entry : wideArgs)
        {
            argv.push_back(entry.c_str());
        }
        argv.push_back(nullptr);

        intptr_t result = _wspawnv(_P_NOWAIT, exe.c_str(), argv.data());
        return result != -1;
    }

} // namespace Fermion
