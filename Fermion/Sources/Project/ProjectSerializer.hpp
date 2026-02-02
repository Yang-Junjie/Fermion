#pragma once

namespace Fermion
{
    class Project;
    class ProjectSerializer
    {
    public:
        ProjectSerializer(std::shared_ptr<Project> project);

        bool serialize(const std::filesystem::path &filepath);

        bool sertializeRuntime(const std::filesystem::path &filepath);

        bool deserialize(const std::filesystem::path &filepath);

        std::string deserializeRuntime(const std::filesystem::path &filepath);

    private:
        std::shared_ptr<Project> m_project;
    };
} // namespace Fermion
