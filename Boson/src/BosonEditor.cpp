#include "Core/Application.hpp"
#include "BosonLayer.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <format>

namespace
{

bool isProjectDescriptor(const std::filesystem::path &path) {
    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return ext == ".fmproj";
}

std::filesystem::path findProjectFileInDirectory(const std::filesystem::path &directory) {
    std::error_code ec;
    for (std::filesystem::directory_iterator it(directory, ec), end; it != end; it.increment(ec)) {
        if (ec)
            break;

        std::error_code entryEc;
        if (!it->is_regular_file(entryEc) || entryEc)
            continue;

        if (isProjectDescriptor(it->path()))
            return it->path();
    }
    return {};
}

std::filesystem::path resolveProjectArgument(const std::filesystem::path &input) {
    if (input.empty())
        return {};

    std::error_code ec;
    std::filesystem::path normalized = input;
    if (!normalized.is_absolute()) {
        normalized = std::filesystem::absolute(normalized, ec);
        if (ec)
            ec.clear();
    }

    if (std::filesystem::is_directory(normalized, ec) && !ec) {
        auto descriptor = findProjectFileInDirectory(normalized);
        if (!descriptor.empty())
            return descriptor;
    }

    ec.clear();
    if (std::filesystem::is_regular_file(normalized, ec) && !ec && isProjectDescriptor(normalized))
        return normalized;

    return {};
}

} // namespace

namespace Fermion
{
class Bonson final : public Application {
public:
    Bonson(const ApplicationSpecification &spec, std::filesystem::path projectPath) : Application(spec) {
        Log::Info("Boson Editor constructor called");
        auto bosonLayer = std::make_unique<BosonLayer>("BosonLayer", projectPath);
        pushLayer(std::move(bosonLayer));
    };

    ~Bonson() override = default;
};

Application *createApplication(int argc, char **argv) {
    std::filesystem::path projectPath;

    if (argc >= 2 && argv[1]) {
        auto resolved = resolveProjectArgument(std::filesystem::path(argv[1]));
        if (!resolved.empty())
            projectPath = resolved;
        else
            Log::Warn(std::format("Unable to resolve project from argument: {}", argv[1]));
    }
    ApplicationSpecification spec;
    spec.name = "Fermion - Boson";
    spec.windowWidth = 1600;
    spec.windowHeight = 900;
    spec.rendererConfig.ShaderPath = "../Boson/Resources/Shaders/";
    Log::Info("start preparing to create the Application");
    return new Fermion::Bonson(spec, projectPath);
}
} // namespace Fermion
