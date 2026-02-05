#pragma once
#include "fmpch.hpp"
#include "Asset/Asset.hpp"
#include "BoneTransform.hpp"
#include <glm/glm.hpp>

namespace Fermion
{
    // Maximum number of bones supported per skeleton
    constexpr uint32_t MAX_BONES = 128;
    // Maximum number of bones that can influence a single vertex
    constexpr uint32_t MAX_BONE_INFLUENCE = 4;

    /**
     * @brief Information about a single bone in a skeleton
     */
    struct BoneInfo
    {
        std::string name;
        int32_t parentIndex = -1;      // -1 indicates root bone
        glm::mat4 offsetMatrix;        // Inverse bind pose matrix
        BoneTransform localBindPose;   // Local bind pose transform
    };

    /**
     * @brief Represents a skeletal hierarchy for skeletal animation
     */
    class Skeleton : public Asset
    {
    public:
        Skeleton() = default;
        ~Skeleton() override = default;

        AssetType getAssetsType() const override { return AssetType::Skeleton; }

        /**
         * @brief Add a bone to the skeleton
         * @param name Bone name
         * @param parentIndex Index of parent bone (-1 for root)
         * @param offsetMatrix Inverse bind pose matrix
         * @param localBindPose Local bind pose transform
         * @return Index of the added bone
         */
        int32_t addBone(const std::string& name, int32_t parentIndex,
                        const glm::mat4& offsetMatrix, const BoneTransform& localBindPose = {});

        /**
         * @brief Find the index of a bone by name
         * @param name Bone name to search for
         * @return Bone index, or -1 if not found
         */
        int32_t findBoneIndex(const std::string& name) const;

        /**
         * @brief Get the total number of bones
         */
        size_t getBoneCount() const { return m_bones.size(); }

        /**
         * @brief Get bone info by index
         */
        const BoneInfo& getBone(size_t index) const { return m_bones[index]; }
        BoneInfo& getBone(size_t index) { return m_bones[index]; }

        /**
         * @brief Get all bones
         */
        const std::vector<BoneInfo>& getBones() const { return m_bones; }

    private:
        std::vector<BoneInfo> m_bones;
        std::unordered_map<std::string, int32_t> m_boneNameMap;
    };
} // namespace Fermion
