#pragma once
namespace Fermion
{
    class AssetManagerPanel
    {
    public:
        AssetManagerPanel() = default;
        ~AssetManagerPanel() = default;

        void onImGuiRender();
    };
}