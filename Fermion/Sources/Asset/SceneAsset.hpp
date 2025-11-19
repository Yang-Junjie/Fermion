#pragma once
#include "Asset.hpp"
#include "Scene/Scene.hpp"
#include "fmpch.hpp"
namespace Fermion
{

    class SceneAsset : public Asset
    {
    public:
        std::shared_ptr<Scene> scene;
        SceneAsset(std::shared_ptr<Scene> scene) : scene(scene)
        {
        }
        virtual AssetType getAssetsType() const override { return AssetType::Scene; }
        virtual ~SceneAsset() = default;
    };
}