#include "LayerStack.hpp"
#include <algorithm>
namespace Fermion
{
    void LayerStack::pushLayer(std::unique_ptr<Layer> layer)
    {
        m_layers.emplace(m_layers.begin() + m_layerInsertIndex, std::move(layer));
        m_layerInsertIndex++;
    }

    void LayerStack::pushOverlay(std::unique_ptr<Layer> overlay)
    {
        m_layers.emplace_back(std::move(overlay));
    }

    void LayerStack::popLayer(Layer *layer)
    {
        auto it = std::find_if(m_layers.begin(), m_layers.begin() + m_layerInsertIndex,
                               [layer](const std::unique_ptr<Layer> &ptr)
                               { return ptr.get() == layer; });
        if (it != m_layers.begin() + m_layerInsertIndex)
        {
            (*it)->OnDetach();
            m_layers.erase(it);
            m_layerInsertIndex--;
        }
    }

    void LayerStack::popOverlay(Layer *overlay)
    {
        auto it = std::find_if(m_layers.begin() + m_layerInsertIndex, m_layers.end(),
                               [overlay](const std::unique_ptr<Layer> &ptr)
                               { return ptr.get() == overlay; });
        if (it != m_layers.end())
        {
            (*it)->OnDetach();
            m_layers.erase(it);
        }
    }
} // namespace Fermion