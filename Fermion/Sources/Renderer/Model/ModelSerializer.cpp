#include "ModelSerializer.hpp"

#include "Core/UUID.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Fermion
{
    AssetMetadata ModelSerializer::serialize(const std::filesystem::path &modelPath,
                                             AssetHandle meshHandle,
                                             const std::vector<AssetHandle> &materials)
    {
        AssetMetadata metadata;
        metadata.Handle = UUID();
        if (static_cast<uint64_t>(metadata.Handle) == 0)
            metadata.Handle = UUID(1);

        metadata.Type = AssetType::ModelSource;
        metadata.FilePath = modelPath;
        metadata.Name = modelPath.has_stem() ? modelPath.stem().string() : "Model";

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Model" << YAML::Value;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << metadata.Name;
            out << YAML::Key << "Mesh" << YAML::Value << static_cast<uint64_t>(meshHandle);

            out << YAML::Key << "Materials" << YAML::Value << YAML::BeginSeq;
            for (size_t slot = 0; slot < materials.size(); ++slot)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Slot" << YAML::Value << static_cast<uint32_t>(slot);
                out << YAML::Key << "Handle" << YAML::Value
                    << static_cast<uint64_t>(materials[slot]);
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;

            out << YAML::EndMap;
        }
        out << YAML::EndMap;

        std::ofstream fout(modelPath);
        if (!fout.is_open())
            return {};

        fout << out.c_str();
        return metadata;
    }

    std::shared_ptr<ModelAsset> ModelSerializer::deserialize(const std::filesystem::path &modelPath,
                                                             AssetHandle handle)
    {
        if (!std::filesystem::exists(modelPath))
            return nullptr;

        try
        {
            YAML::Node data = YAML::LoadFile(modelPath.string());
            if (!data["Model"])
                return nullptr;

            YAML::Node modelNode = data["Model"];
            auto model = std::make_shared<ModelAsset>();

            if (auto n = modelNode["Mesh"]; n)
                model->mesh = AssetHandle(n.as<uint64_t>());

            if (auto matsNode = modelNode["Materials"]; matsNode && matsNode.IsSequence())
            {
                size_t maxSlot = 0;
                for (const auto &entry : matsNode)
                {
                    if (entry.IsMap() && entry["Slot"])
                        maxSlot = std::max(maxSlot, static_cast<size_t>(entry["Slot"].as<uint32_t>()));
                }

                model->materials.assign(maxSlot + 1, AssetHandle(0));
                for (const auto &entry : matsNode)
                {
                    if (entry.IsMap())
                    {
                        uint32_t slot = entry["Slot"] ? entry["Slot"].as<uint32_t>() : 0;
                        if (entry["Handle"])
                        {
                            if (slot >= model->materials.size())
                                model->materials.resize(slot + 1, AssetHandle(0));
                            model->materials[slot] = AssetHandle(entry["Handle"].as<uint64_t>());
                        }
                    }
                    else
                    {
                        model->materials.push_back(AssetHandle(entry.as<uint64_t>()));
                    }
                }
            }

            model->handle = handle;
            return model;
        }
        catch (...)
        {
            return nullptr;
        }
    }

} // namespace Fermion
