#include "MeshImporter.hpp"
#include "Core/UUID.hpp"
#include "Asset/AssetSerializer.hpp"

namespace Fermion {
AssetMetadata MeshImporter::importAsset(const std::filesystem::path &assetPath) {
    m_Metadata.Handle = UUID();
    if (static_cast<uint64_t>(m_Metadata.Handle) == 0)
        m_Metadata.Handle = UUID(1);

    m_Metadata.Type = AssetType::Mesh;
    m_Metadata.FilePath = assetPath;
    m_Metadata.Name = assetPath.stem().string();

    return m_Metadata;
}
} // namespace Fermion
