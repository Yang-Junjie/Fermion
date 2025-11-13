#include "fmpch.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Core/Engine.hpp"
#include <windows.h>
#include <prsht.h>
#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Fermion
{

	float Time::getTime()
	{
		return glfwGetTime();
	}

	std::string FileDialogs::openFile(const char *filter, std::string defaultPath)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = {0};
		CHAR currentDir[256] = {0};
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow *)Engine::get().getWindow().getNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		// if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = defaultPath.c_str();

		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();
	}

	std::string FileDialogs::saveFile(const char *filter, std::string defaultPath)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = {0};
		CHAR currentDir[256] = {0};
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow *)Engine::get().getWindow().getNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		// if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = defaultPath.c_str();
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		// Sets the default extension by extracting it from the filter
		ofn.lpstrDefExt = strchr(filter, '\0') + 1;

		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();
	}

}
