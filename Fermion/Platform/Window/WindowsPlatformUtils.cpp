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


	std::filesystem::path FileDialogs::openFile(const char *filter, std::string defaultPath)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = {0};
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(
			(GLFWwindow *)Engine::get().getWindow().getNativeWindow());
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
			(GLFWwindow *)Engine::get().getWindow().getNativeWindow());
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

}
