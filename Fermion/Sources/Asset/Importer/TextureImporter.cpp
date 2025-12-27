#include "TextureImporter.hpp"
#include "Core/UUID.hpp"
#include "Asset/AssetSerializer.hpp"

namespace Fermion {
AssetMetadata TextureImporter::importAsset(const std::filesystem::path &assetPath) {
    m_Metadata.Handle = UUID();
    if (static_cast<uint64_t>(m_Metadata.Handle) == 0)
        m_Metadata.Handle = UUID(1);

    m_Metadata.Type = AssetType::Texture;
    m_Metadata.FilePath = assetPath;
    m_Metadata.Name = assetPath.stem().string();

    return m_Metadata;
}
} // namespace Fermion
