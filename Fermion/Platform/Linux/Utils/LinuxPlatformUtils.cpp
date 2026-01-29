#include "fmpch.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Core/Application.hpp"
#include <GLFW/glfw3.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>

namespace Fermion
{

    float Time::getTime()
    {
        return glfwGetTime();
    }

    // Helper function to execute zenity command and get result
    static std::string executeCommand(const std::string& command)
    {
        std::string result;
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe)
            return result;

        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            result += buffer;
        }
        pclose(pipe);

        // Remove trailing newline
        if (!result.empty() && result.back() == '\n')
            result.pop_back();

        return result;
    }

    static std::string convertFilterToZenity(const char* filter)
    {
        if (!filter || filter[0] == '\0')
            return "";

        std::string result = "--file-filter='";
        std::string filterStr(filter);

        // Find the description and pattern
        size_t nullPos = filterStr.find('\0');
        if (nullPos != std::string::npos && nullPos + 1 < filterStr.length())
        {
            std::string description = filterStr.substr(0, nullPos);
            std::string pattern = filterStr.substr(nullPos + 1);

            // Remove second null terminator if present
            size_t secondNull = pattern.find('\0');
            if (secondNull != std::string::npos)
                pattern = pattern.substr(0, secondNull);

            result += description + " | " + pattern;
        }
        else
        {
            result += "All Files | *";
        }

        result += "'";
        return result;
    }

    std::filesystem::path FileDialogs::openFile(const char *filter, std::string defaultPath)
    {
        std::string command = "zenity --file-selection --title='Open File'";

        if (!defaultPath.empty())
        {
            command += " --filename='" + defaultPath + "/'";
        }

        std::string filterArg = convertFilterToZenity(filter);
        if (!filterArg.empty())
        {
            command += " " + filterArg;
        }

        std::string result = executeCommand(command);
        if (!result.empty())
            return std::filesystem::path(result);

        return {};
    }

    std::filesystem::path FileDialogs::saveFile(const char *filter, std::string defaultPath)
    {
        std::string command = "zenity --file-selection --save --confirm-overwrite --title='Save File'";

        if (!defaultPath.empty())
        {
            command += " --filename='" + defaultPath + "/'";
        }

        std::string filterArg = convertFilterToZenity(filter);
        if (!filterArg.empty())
        {
            command += " " + filterArg;
        }

        std::string result = executeCommand(command);
        if (!result.empty())
            return std::filesystem::path(result);

        return {};
    }

    std::filesystem::path FileDialogs::selectDirectory(std::string defaultPath)
    {
        std::string command = "zenity --file-selection --directory --title='Select Directory'";

        if (!defaultPath.empty())
        {
            command += " --filename='" + defaultPath + "/'";
        }

        std::string result = executeCommand(command);
        if (!result.empty())
            return std::filesystem::path(result);

        return {};
    }

    bool Process::launchDetached(const std::filesystem::path &executablePath, const std::vector<std::string> &arguments)
    {
        if (executablePath.empty())
            return false;

        pid_t pid = fork();

        if (pid < 0)
        {
            return false;
        }
        else if (pid == 0)
        {
            std::vector<char*> argv;

            std::string exePath = executablePath.string();
            argv.push_back(const_cast<char*>(exePath.c_str()));

            std::vector<std::string> argStrings = arguments;
            for (auto& arg : argStrings)
            {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);

            execv(exePath.c_str(), argv.data());

            exit(1);
        }
        else
        {
            return true;
        }
    }

} // namespace Fermion
