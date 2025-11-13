#pragma once

#include <string>

namespace Fermion {

	class FileDialogs
	{
	public:
		static std::string openFile(const char* filter,std::string defaultPath);
		static std::string saveFile(const char* filter,std::string defaultPath);
	};

	class Time
	{
	public:
		static float getTime();
	};

}
