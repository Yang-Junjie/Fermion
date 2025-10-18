#pragma once
#include "Layer.hpp"
#include <vector>
#include <memory>
namespace Fermion
{
    class LayerStack
    {
    public:
        LayerStack() = default;
        ~LayerStack() = default;

        // 因为 LayerStack 负责管理 Layer 的生命周期，所以这里使用 unique_ptr
        void pushLayer(std::unique_ptr<Layer> layer);
        void pushOverlay(std::unique_ptr<Layer> overlay);
        void popLayer(Layer *layer);
        void popOverlay(Layer *overlay);

        std::vector<std::unique_ptr<Layer>>::iterator begin() { return m_layers.begin(); }
        std::vector<std::unique_ptr<Layer>>::iterator end() { return m_layers.end(); }

        std::vector<std::unique_ptr<Layer>>::reverse_iterator rbegin() { return m_layers.rbegin(); }
        std::vector<std::unique_ptr<Layer>>::reverse_iterator rend() { return m_layers.rend(); }

        std::vector<std::unique_ptr<Layer>>::const_iterator cbegin() const { return m_layers.begin(); }
        std::vector<std::unique_ptr<Layer>>::const_iterator cend() const { return m_layers.end(); }

        std::vector<std::unique_ptr<Layer>>::const_reverse_iterator crbegin() const { return m_layers.rbegin(); }
        std::vector<std::unique_ptr<Layer>>::const_reverse_iterator crend() const { return m_layers.rend(); }

    private:
        std::vector<std::unique_ptr<Layer>> m_layers;
        unsigned int m_layerInsertIndex = 0;
    };
}
