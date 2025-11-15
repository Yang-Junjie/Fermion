#pragma once

#include <string>
#include <filesystem>

namespace Fermion {

	class FileDialogs
	{
	public:
		static std::filesystem::path openFile(const char* filter,std::string defaultPath);
		static std::filesystem::path saveFile(const char* filter,std::string defaultPath);
	};

	class Time
	{
	public:
		static float getTime();
	};

}
