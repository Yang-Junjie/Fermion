#include "fmpch.hpp"

#include "ProjectSerializer.hpp"

#include "Project.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>
#include <system_error>

namespace Fermion
{
    namespace
    {
        std::string make_relative(const std::filesystem::path &path, const std::filesystem::path &baseDir)
        {
            if (path.empty())
                return {};

            std::error_code ec;
            auto rel = std::filesystem::relative(path, baseDir, ec);
            if (ec)
            {
                return path.generic_string();
            }

            return rel.generic_string();
        }

        std::filesystem::path make_absolute(const std::string &stored, const std::filesystem::path &baseDir)
        {
            if (stored.empty())
                return {};

            std::filesystem::path p = stored;
            if (p.is_absolute())
                return p.lexically_normal();

            return (baseDir / p).lexically_normal();
        }
    } // namespace

    ProjectSerializer::ProjectSerializer(std::shared_ptr<Project> project) : m_project(project)
    {
    }

    bool ProjectSerializer::serialize(const std::filesystem::path &filepath)
    {
        const auto &config = m_project->getConfig();
        const auto projectDir = filepath.parent_path();

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Project" << YAML::Value;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << config.name;
            out << YAML::Key << "Version" << YAML::Value << config.version;
            out << YAML::Key << "Author" << YAML::Value << config.author;
            out << YAML::EndMap;
        }

        out << YAML::Key << "Directories" << YAML::Value;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "AssetDirectory"
                << YAML::Value << make_relative(config.assetDirectory, projectDir);
            out << YAML::Key << "ScriptDirectory"
                << YAML::Value << make_relative(config.scriptDirectory, projectDir);
            out << YAML::EndMap;
        }

        out << YAML::Key << "Runtime" << YAML::Value;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "StartScene"
                << YAML::Value << make_relative(config.startScene, projectDir);
            out << YAML::EndMap;
        }

        out << YAML::EndMap;

        std::ofstream fout(filepath);
        fout << out.c_str();

        return true;
    }

    bool ProjectSerializer::deserialize(const std::filesystem::path &filepath)
    {
        if (!std::filesystem::exists(filepath))
        {
            Log::Error("Project file does not exist: " + filepath.string());
            return false;
        }

        YAML::Node data = YAML::LoadFile(filepath.string());
        if (!data)
        {
            Log::Error("Failed to load project YAML: " + filepath.string());
            return false;
        }

        const auto projectDir = filepath.parent_path();
        ProjectConfig &config = m_project->getConfig();

        if (data["Project"])
        {
            YAML::Node projectNode = data["Project"];
            if (projectNode["Name"])
                config.name = projectNode["Name"].as<std::string>();
            if (projectNode["Version"])
                config.version = projectNode["Version"].as<std::string>();
            if (projectNode["Author"])
                config.author = projectNode["Author"].as<std::string>();

            if (projectNode["AssetDirectory"] && !data["Directories"])
                config.assetDirectory = make_absolute(projectNode["AssetDirectory"].as<std::string>(), projectDir);
            if (projectNode["StartScene"] && !data["Runtime"])
                config.startScene = make_absolute(projectNode["StartScene"].as<std::string>(), projectDir);
        }

        if (data["Directories"])
        {
            YAML::Node dirNode = data["Directories"];
            if (dirNode["AssetDirectory"])
                config.assetDirectory = make_absolute(dirNode["AssetDirectory"].as<std::string>(), projectDir);
            if (dirNode["ScriptDirectory"])
                config.scriptDirectory = make_absolute(dirNode["ScriptDirectory"].as<std::string>(), projectDir);
        }

        if (config.scriptDirectory.empty() && !config.assetDirectory.empty())
            config.scriptDirectory = (config.assetDirectory / "scripts").lexically_normal();

        if (data["Runtime"])
        {
            YAML::Node runtimeNode = data["Runtime"];
            if (runtimeNode["StartScene"])
                config.startScene = make_absolute(runtimeNode["StartScene"].as<std::string>(), projectDir);
        }

        return true;
    }
} // namespace Fermion
