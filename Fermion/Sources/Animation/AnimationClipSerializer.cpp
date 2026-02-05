#include "AnimationClipSerializer.hpp"
#include <fstream>

namespace Fermion
{
    struct AnimationClipBinaryHeader
    {
        uint32_t magic = 0x414E494D;  // "ANIM" in ASCII
        uint32_t version = 1;
        uint32_t nameLength = 0;
        float duration = 0.0f;
        float ticksPerSecond = 25.0f;
        uint32_t channelCount = 0;
    };

    struct BoneChannelHeader
    {
        uint32_t boneNameLength;
        uint32_t positionKeyCount;
        uint32_t rotationKeyCount;
        uint32_t scaleKeyCount;
    };

    bool AnimationClipSerializer::serialize(const std::filesystem::path &filepath, const AnimationClip &clip)
    {
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open())
            return false;

        AnimationClipBinaryHeader header;
        header.nameLength = static_cast<uint32_t>(clip.getName().size());
        header.duration = clip.getDuration();
        header.ticksPerSecond = clip.getTicksPerSecond();
        header.channelCount = static_cast<uint32_t>(clip.getChannels().size());

        file.write(reinterpret_cast<const char *>(&header), sizeof(header));
        file.write(clip.getName().data(), header.nameLength);

        for (const auto &channel : clip.getChannels())
        {
            BoneChannelHeader chanHeader;
            chanHeader.boneNameLength = static_cast<uint32_t>(channel.boneName.size());
            chanHeader.positionKeyCount = static_cast<uint32_t>(channel.positionKeys.size());
            chanHeader.rotationKeyCount = static_cast<uint32_t>(channel.rotationKeys.size());
            chanHeader.scaleKeyCount = static_cast<uint32_t>(channel.scaleKeys.size());

            file.write(reinterpret_cast<const char *>(&chanHeader), sizeof(chanHeader));
            file.write(channel.boneName.data(), chanHeader.boneNameLength);

            // Position keys
            for (const auto &key : channel.positionKeys)
            {
                file.write(reinterpret_cast<const char *>(&key.time), sizeof(float));
                file.write(reinterpret_cast<const char *>(&key.value), sizeof(glm::vec3));
            }

            // Rotation keys
            for (const auto &key : channel.rotationKeys)
            {
                file.write(reinterpret_cast<const char *>(&key.time), sizeof(float));
                file.write(reinterpret_cast<const char *>(&key.value), sizeof(glm::quat));
            }

            // Scale keys
            for (const auto &key : channel.scaleKeys)
            {
                file.write(reinterpret_cast<const char *>(&key.time), sizeof(float));
                file.write(reinterpret_cast<const char *>(&key.value), sizeof(glm::vec3));
            }
        }

        file.close();
        return true;
    }

    std::shared_ptr<AnimationClip> AnimationClipSerializer::deserialize(const std::filesystem::path &filepath,
                                                                         AssetHandle handle)
    {
        if (!std::filesystem::exists(filepath))
            return nullptr;

        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open())
            return nullptr;

        AnimationClipBinaryHeader header;
        file.read(reinterpret_cast<char *>(&header), sizeof(header));

        if (header.magic != 0x414E494D || header.version != 1)
            return nullptr;

        std::string name(header.nameLength, '\0');
        file.read(name.data(), header.nameLength);

        auto clip = std::make_shared<AnimationClip>(name, header.duration, header.ticksPerSecond);
        clip->handle = handle;

        for (uint32_t i = 0; i < header.channelCount; i++)
        {
            BoneChannelHeader chanHeader;
            file.read(reinterpret_cast<char *>(&chanHeader), sizeof(chanHeader));

            BoneChannel channel;
            channel.boneName.resize(chanHeader.boneNameLength);
            file.read(channel.boneName.data(), chanHeader.boneNameLength);

            // Position keys
            channel.positionKeys.resize(chanHeader.positionKeyCount);
            for (auto &key : channel.positionKeys)
            {
                file.read(reinterpret_cast<char *>(&key.time), sizeof(float));
                file.read(reinterpret_cast<char *>(&key.value), sizeof(glm::vec3));
            }

            // Rotation keys
            channel.rotationKeys.resize(chanHeader.rotationKeyCount);
            for (auto &key : channel.rotationKeys)
            {
                file.read(reinterpret_cast<char *>(&key.time), sizeof(float));
                file.read(reinterpret_cast<char *>(&key.value), sizeof(glm::quat));
            }

            // Scale keys
            channel.scaleKeys.resize(chanHeader.scaleKeyCount);
            for (auto &key : channel.scaleKeys)
            {
                file.read(reinterpret_cast<char *>(&key.time), sizeof(float));
                file.read(reinterpret_cast<char *>(&key.value), sizeof(glm::vec3));
            }

            clip->addChannel(std::move(channel));
        }

        file.close();
        return clip;
    }
} // namespace Fermion
