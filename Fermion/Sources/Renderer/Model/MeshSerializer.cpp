#include "MeshSerializer.hpp"
#include <fstream>

namespace Fermion
{
    // 二进制文件格式定义
    struct MeshBinaryHeader
    {
        uint32_t magic = 0x4D455348;      // "MESH" in ASCII
        uint32_t version = 2;             // Version 2 adds bone data support
        uint32_t vertexCount;
        uint32_t indexCount;
        uint32_t subMeshCount;
        uint32_t boneDataCount;           // New: bone data count (0 if not skinned)
        AABB boundingBox;
        uint32_t nameLength;              //暂时保留为0
    };

    bool MeshSerializer::serialize(const std::filesystem::path &filepath, const Mesh &mesh,
                                   const MeshSerializeOptions &options)
    {
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open())
        {

            return false;
        }

        MeshBinaryHeader header;
        header.vertexCount = static_cast<uint32_t>(mesh.getVertices().size());
        header.indexCount = static_cast<uint32_t>(mesh.getIndices().size());
        header.subMeshCount = static_cast<uint32_t>(mesh.getSubMeshes().size());
        header.boneDataCount = static_cast<uint32_t>(mesh.getBoneData().size());
        header.boundingBox = mesh.getBoundingBox();
        header.nameLength = 0;


        file.write(reinterpret_cast<const char*>(&header), sizeof(header));

        if (header.vertexCount > 0)
        {
            file.write(reinterpret_cast<const char*>(mesh.getVertices().data()),
                      header.vertexCount * sizeof(Vertex));
        }

        if (header.indexCount > 0)
        {
            file.write(reinterpret_cast<const char*>(mesh.getIndices().data()),
                      header.indexCount * sizeof(uint32_t));
        }

        if (header.subMeshCount > 0)
        {
            file.write(reinterpret_cast<const char*>(mesh.getSubMeshes().data()),
                      header.subMeshCount * sizeof(SubMesh));
        }

        // Write bone data if present
        if (header.boneDataCount > 0)
        {
            file.write(reinterpret_cast<const char*>(mesh.getBoneData().data()),
                      header.boneDataCount * sizeof(VertexBoneData));
        }

        file.close();

        return true;
    }

    std::shared_ptr<Mesh> MeshSerializer::deserialize(const std::filesystem::path &filepath,
                                                      AssetHandle handle)
    {
        if (!std::filesystem::exists(filepath))
        {
            return nullptr;
        }

        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open())
        {
            return nullptr;
        }

        MeshBinaryHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));

        if (header.magic != 0x4D455348)
        {
            return nullptr;
        }

        // Support both version 1 and 2
        if (header.version != 1 && header.version != 2)
        {
            return nullptr;
        }

        std::vector<Vertex> vertices(header.vertexCount);
        if (header.vertexCount > 0)
        {
            file.read(reinterpret_cast<char*>(vertices.data()),
                     header.vertexCount * sizeof(Vertex));
        }

        std::vector<uint32_t> indices(header.indexCount);
        if (header.indexCount > 0)
        {
            file.read(reinterpret_cast<char*>(indices.data()),
                     header.indexCount * sizeof(uint32_t));
        }


        std::vector<SubMesh> subMeshes(header.subMeshCount);
        if (header.subMeshCount > 0)
        {
            file.read(reinterpret_cast<char*>(subMeshes.data()),
                     header.subMeshCount * sizeof(SubMesh));
        }

        // Read bone data if version 2 and has bone data
        std::vector<VertexBoneData> boneData;
        if (header.version >= 2 && header.boneDataCount > 0)
        {
            boneData.resize(header.boneDataCount);
            file.read(reinterpret_cast<char*>(boneData.data()),
                     header.boneDataCount * sizeof(VertexBoneData));
        }

        file.close();

        auto mesh = std::make_shared<Mesh>(std::move(vertices),
                                           std::move(indices),
                                           std::move(subMeshes));
        mesh->handle = handle;

        // Set bone data if present
        if (!boneData.empty())
        {
            mesh->setBoneData(std::move(boneData));
        }


        return mesh;
    }

} // namespace Fermion
