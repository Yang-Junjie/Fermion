#pragma once

#include "Scene.hpp"
#include <filesystem>

namespace Fermion
{
    class SceneSerializer
    {
    public:
        SceneSerializer(const std::shared_ptr<Scene> &scene);

        void serialize(const std::filesystem::path &filepath);

        void serializeRuntime(const std::filesystem::path &filepath);

        bool deserialize(const std::filesystem::path &filepath);

        bool deserializeRuntime(const std::filesystem::path &filepath);

    private:
        std::shared_ptr<Scene> m_scene;
    };
} // namespace Fermion
