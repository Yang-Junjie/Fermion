#include "RenderGraphResource.hpp"

namespace Fermion
{
    RenderGraphResource::RenderGraphResource(const std::string &name, const RenderGraphResourceDesc &desc)
        : m_Name(name), m_Desc(desc), m_IsExternal(false)
    {
        while (!m_Handle.isValid())
            m_Handle = RenderGraphResourceHandle{};
    }

    RenderGraphResource::RenderGraphResource(const std::string &name, std::shared_ptr<Framebuffer> external)
        : m_Name(name), m_Framebuffer(external), m_IsExternal(true)
    {
        while (!m_Handle.isValid())
            m_Handle = RenderGraphResourceHandle{};

        if (external)
        {
            const auto &spec = external->getSpecification();
            m_Desc.width = spec.width;
            m_Desc.height = spec.height;
            m_Desc.isTransient = false;
        }
    }

} // namespace Fermion
