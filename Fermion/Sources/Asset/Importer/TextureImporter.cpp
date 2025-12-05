#include "TextureImporter.hpp"
#include "Core/UUID.hpp"
#include "Asset/AssetSerialzer.hpp"

namespace Fermion
{
    AssetMetadata TextureImporter::importAsset(const std::filesystem::path &assetPath)
    {
        // Create basic metadata for a texture asset
        m_Metadata.Handle = UUID();
        if (static_cast<uint64_t>(m_Metadata.Handle) == 0)
            m_Metadata.Handle = UUID(1);

        m_Metadata.Type = AssetType::Texture;
        m_Metadata.FilePath = assetPath;
        m_Metadata.Name = assetPath.stem().string();
        m_Metadata.isMemoryAsset = false;

        return m_Metadata;
    }

    void TextureImporter::writeMetadata(const AssetMetadata &metadata)
    {
        // Meta file is simply the asset path with ".fmasset" appended
        std::filesystem::path metaPath = metadata.FilePath;
        metaPath += ".fmasset";
        AssetSerializer::serializeMeta(metaPath, metadata.Handle, metadata.Type);
    }

    void TextureImporter::loadMetadata(const std::filesystem::path &metaFilePath)
    {
        AssetHandle handle;
        AssetType type;
        if (!AssetSerializer::deserializeMeta(metaFilePath, handle, type))
        {
            m_Metadata = AssetMetadata{};
            return;
        }

        std::filesystem::path assetPath = metaFilePath;
        // Remove the ".fmasset" extension to get back the asset path
        assetPath.replace_extension();

        m_Metadata.Handle = handle;
        m_Metadata.Type = type;
        m_Metadata.FilePath = assetPath;
        m_Metadata.Name = assetPath.stem().string();
        m_Metadata.isMemoryAsset = false;
    }

}

