#include "SkeletonSerializer.hpp"
#include <fstream>

namespace Fermion
{
    struct SkeletonBinaryHeader
    {
        uint32_t magic = 0x534B454C;  // "SKEL" in ASCII
        uint32_t version = 1;
        uint32_t boneCount = 0;
    };

    struct BoneInfoBinary
    {
        uint32_t nameLength;
        int32_t parentIndex;
        glm::mat4 offsetMatrix;
        glm::vec3 bindPosition;
        glm::quat bindRotation;
        glm::vec3 bindScale;
    };

    bool SkeletonSerializer::serialize(const std::filesystem::path &filepath, const Skeleton &skeleton)
    {
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open())
            return false;

        SkeletonBinaryHeader header;
        header.boneCount = static_cast<uint32_t>(skeleton.getBoneCount());

        file.write(reinterpret_cast<const char *>(&header), sizeof(header));

        for (size_t i = 0; i < skeleton.getBoneCount(); i++)
        {
            const auto &bone = skeleton.getBone(i);

            BoneInfoBinary boneData;
            boneData.nameLength = static_cast<uint32_t>(bone.name.size());
            boneData.parentIndex = bone.parentIndex;
            boneData.offsetMatrix = bone.offsetMatrix;
            boneData.bindPosition = bone.localBindPose.position;
            boneData.bindRotation = bone.localBindPose.rotation;
            boneData.bindScale = bone.localBindPose.scale;

            file.write(reinterpret_cast<const char *>(&boneData), sizeof(boneData));
            file.write(bone.name.data(), boneData.nameLength);
        }

        file.close();
        return true;
    }

    std::shared_ptr<Skeleton> SkeletonSerializer::deserialize(const std::filesystem::path &filepath,
                                                               AssetHandle handle)
    {
        if (!std::filesystem::exists(filepath))
            return nullptr;

        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open())
            return nullptr;

        SkeletonBinaryHeader header;
        file.read(reinterpret_cast<char *>(&header), sizeof(header));

        if (header.magic != 0x534B454C || header.version != 1)
            return nullptr;

        auto skeleton = std::make_shared<Skeleton>();
        skeleton->handle = handle;

        for (uint32_t i = 0; i < header.boneCount; i++)
        {
            BoneInfoBinary boneData;
            file.read(reinterpret_cast<char *>(&boneData), sizeof(boneData));

            std::string name(boneData.nameLength, '\0');
            file.read(name.data(), boneData.nameLength);

            BoneTransform bindPose(boneData.bindPosition, boneData.bindRotation, boneData.bindScale);
            skeleton->addBone(name, boneData.parentIndex, boneData.offsetMatrix, bindPose);
        }

        file.close();
        return skeleton;
    }
} // namespace Fermion
