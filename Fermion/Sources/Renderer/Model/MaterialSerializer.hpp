#pragma once
#include "Material.hpp"

#include <filesystem>
#include <memory>

namespace Fermion {

struct MaterialSerializeOptions {
    bool includePBRMaps = false;
    bool includeEditorData = false;
};

class MaterialSerializer {
public:
    static bool serialize(const std::filesystem::path &filepath, const Material &material,
                          const MaterialSerializeOptions &options = {});

    static std::shared_ptr<Material> deserialize(const std::filesystem::path &filepath,
                                                 AssetHandle handle = AssetHandle(0));
};

} // namespace Fermion
