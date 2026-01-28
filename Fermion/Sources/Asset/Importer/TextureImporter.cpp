#include "TextureImporter.hpp"
#include "Core/UUID.hpp"
#include "Asset/AssetSerializer.hpp"
#include "Project/Project.hpp"
#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Fermion {

namespace {
    std::string FilterModeToString(TextureFilterMode mode) {
        switch (mode) {
        case TextureFilterMode::Nearest: return "Nearest";
        case TextureFilterMode::Linear: return "Linear";
        }
        return "Linear";
    }

    TextureFilterMode StringToFilterMode(const std::string &str) {
        if (str == "Nearest") return TextureFilterMode::Nearest;
        return TextureFilterMode::Linear;
    }

    std::string WrapModeToString(TextureWrapMode mode) {
        switch (mode) {
        case TextureWrapMode::Repeat: return "Repeat";
        case TextureWrapMode::ClampToEdge: return "ClampToEdge";
        case TextureWrapMode::MirroredRepeat: return "MirroredRepeat";
        }
        return "Repeat";
    }

    TextureWrapMode StringToWrapMode(const std::string &str) {
        if (str == "ClampToEdge") return TextureWrapMode::ClampToEdge;
        if (str == "MirroredRepeat") return TextureWrapMode::MirroredRepeat;
        return TextureWrapMode::Repeat;
    }

    std::string PathRelativeToProject(const std::filesystem::path &absolutePath) {
        if (absolutePath.empty())
            return {};

        auto normalized = absolutePath.lexically_normal();
        if (Project::getActive()) {
            auto projectDir = Project::getProjectDirectory();
            if (!projectDir.empty()) {
                std::error_code ec;
                auto relative = std::filesystem::relative(normalized, projectDir, ec);
                if (!ec)
                    return relative.generic_string();
            }
        }
        return normalized.generic_string();
    }

    std::filesystem::path ResolveStoredPath(const std::string &stored, const std::filesystem::path &ftexPath) {
        if (stored.empty())
            return {};

        std::filesystem::path storedPath = std::filesystem::path(stored).lexically_normal();
        if (storedPath.is_absolute())
            return storedPath;

        // 优先使用项目目录
        if (Project::getActive()) {
            auto projectDir = Project::getProjectDirectory();
            if (!projectDir.empty())
                return (projectDir / storedPath).lexically_normal();
        }

        // Fallback: 如果存储的路径以 "Assets/" 开头，尝试从 ftexPath 推断项目目录
        std::string storedStr = stored;
        if (storedStr.find("Assets/") == 0 || storedStr.find("Assets\\") == 0) {
            // 从 ftexPath 向上查找 "Assets" 目录
            auto current = ftexPath.parent_path();
            while (!current.empty() && current.has_parent_path()) {
                if (current.filename() == "Assets") {
                    auto projectDir = current.parent_path();
                    return (projectDir / storedPath).lexically_normal();
                }
                current = current.parent_path();
            }
        }

        // 最后的 fallback: 相对于 ftex 文件所在目录
        // 但只使用存储路径的文件名部分
        return (ftexPath.parent_path() / storedPath.filename()).lexically_normal();
    }
}

AssetMetadata TextureImporter::importAsset(const std::filesystem::path &assetPath) {
    m_Metadata.Handle = UUID();
    if (static_cast<uint64_t>(m_Metadata.Handle) == 0)
        m_Metadata.Handle = UUID(1);

    m_Metadata.Type = AssetType::Texture;
    m_Metadata.FilePath = assetPath;
    m_Metadata.Name = assetPath.stem().string();

    return m_Metadata;
}

void TextureImporter::generateDefaultFtex(const std::filesystem::path &imagePath) {
    auto ftexPath = imagePath;
    ftexPath.replace_extension(".ftex");

    if (std::filesystem::exists(ftexPath))
        return;

    TextureAssetSpecification spec;
    spec.SourcePath = imagePath;
    spec.GenerateMipmaps = true;
    spec.MinFilter = TextureFilterMode::Linear;
    spec.MagFilter = TextureFilterMode::Linear;
    spec.WrapS = TextureWrapMode::Repeat;
    spec.WrapT = TextureWrapMode::Repeat;
    spec.Anisotropy = 1.0f;
    // 默认不使用 sRGB，保持与原来的行为一致
    // 用户可以手动为 albedo/diffuse 贴图启用 sRGB
    spec.sRGB = false;

    serializeFtex(ftexPath, spec);

    // 如果原始图片有 .meta 文件，继承其 Handle 以保持向后兼容
    auto imageMetaPath = imagePath;
    imageMetaPath += ".meta";
    if (std::filesystem::exists(imageMetaPath)) {
        try {
            YAML::Node data = YAML::LoadFile(imageMetaPath.string());
            auto assetNode = data["Asset"];
            if (assetNode && assetNode["Handle"]) {
                uint64_t oldHandle = assetNode["Handle"].as<uint64_t>();
                if (oldHandle != 0) {
                    // 为 .ftex 创建 meta 文件，使用原始图片的 Handle
                    auto ftexMetaPath = ftexPath;
                    ftexMetaPath += ".meta";

                    YAML::Emitter out;
                    out << YAML::BeginMap;
                    out << YAML::Key << "Asset" << YAML::Value;
                    {
                        out << YAML::BeginMap;
                        out << YAML::Key << "Handle" << YAML::Value << oldHandle;
                        out << YAML::Key << "Type" << YAML::Value << "Texture";
                        out << YAML::Key << "Name" << YAML::Value << ftexPath.stem().string();
                        out << YAML::Key << "FilePath" << YAML::Value << PathRelativeToProject(ftexPath);
                        out << YAML::EndMap;
                    }
                    out << YAML::EndMap;

                    std::ofstream fout(ftexMetaPath);
                    fout << out.c_str();

                    Log::Info(std::format("TextureImporter: Migrated texture Handle {} from {} to {}",
                        oldHandle, imagePath.filename().string(), ftexPath.filename().string()));
                }
            }
        } catch (...) {
            // 忽略解析错误
        }
    }
}

void TextureImporter::serializeFtex(const std::filesystem::path &ftexPath, const TextureAssetSpecification &spec) {
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "TextureAsset" << YAML::Value;
    {
        out << YAML::BeginMap;
        out << YAML::Key << "SourcePath" << YAML::Value << PathRelativeToProject(spec.SourcePath);
        out << YAML::Key << "GenerateMipmaps" << YAML::Value << spec.GenerateMipmaps;
        out << YAML::Key << "MinFilter" << YAML::Value << FilterModeToString(spec.MinFilter);
        out << YAML::Key << "MagFilter" << YAML::Value << FilterModeToString(spec.MagFilter);
        out << YAML::Key << "WrapS" << YAML::Value << WrapModeToString(spec.WrapS);
        out << YAML::Key << "WrapT" << YAML::Value << WrapModeToString(spec.WrapT);
        out << YAML::Key << "Anisotropy" << YAML::Value << spec.Anisotropy;
        out << YAML::Key << "sRGB" << YAML::Value << spec.sRGB;
        out << YAML::EndMap;
    }
    out << YAML::EndMap;

    std::ofstream fout(ftexPath);
    fout << out.c_str();
}

TextureAssetSpecification TextureImporter::deserializeFtex(const std::filesystem::path &ftexPath) {
    TextureAssetSpecification spec;

    if (!std::filesystem::exists(ftexPath))
        return spec;

    try {
        YAML::Node data = YAML::LoadFile(ftexPath.string());
        auto node = data["TextureAsset"];
        if (!node)
            return spec;

        if (node["SourcePath"]) {
            std::string storedPath = node["SourcePath"].as<std::string>();
            spec.SourcePath = ResolveStoredPath(storedPath, ftexPath);
        }

        if (node["GenerateMipmaps"])
            spec.GenerateMipmaps = node["GenerateMipmaps"].as<bool>();

        if (node["MinFilter"])
            spec.MinFilter = StringToFilterMode(node["MinFilter"].as<std::string>());

        if (node["MagFilter"])
            spec.MagFilter = StringToFilterMode(node["MagFilter"].as<std::string>());

        if (node["WrapS"])
            spec.WrapS = StringToWrapMode(node["WrapS"].as<std::string>());

        if (node["WrapT"])
            spec.WrapT = StringToWrapMode(node["WrapT"].as<std::string>());

        if (node["Anisotropy"])
            spec.Anisotropy = node["Anisotropy"].as<float>();

        if (node["sRGB"])
            spec.sRGB = node["sRGB"].as<bool>();

    } catch (...) {
        // 解析失败，返回默认值
    }

    return spec;
}

} // namespace Fermion
