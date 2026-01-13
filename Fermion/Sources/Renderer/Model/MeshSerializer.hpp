#pragma once
#include "Mesh.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace Fermion {

struct MeshSerializeOptions {
    std::string nameOverride;
};

class MeshSerializer {
public:
    static bool serialize(const std::filesystem::path &filepath, const Mesh &mesh,
                          const MeshSerializeOptions &options = {});

    static std::shared_ptr<Mesh> deserialize(const std::filesystem::path &filepath,
                                             AssetHandle handle = AssetHandle(0));
};

} // namespace Fermion
