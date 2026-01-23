#pragma once
#include "RenderGraphResource.hpp"
#include "Renderer/CommandBuffer.hpp"
#include <functional>
#include <memory>
#include <unordered_map>

namespace Fermion
{
    class PassContext
    {
    public:
        CommandBuffer &commands;

        PassContext(CommandBuffer &cmd,
                    std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<Framebuffer>> &resources)
            : commands(cmd), m_Resources(resources)
        {
        }

        std::shared_ptr<Framebuffer> getFramebuffer(RenderGraphResourceHandle handle) const
        {
            auto it = m_Resources.find(handle);
            return it != m_Resources.end() ? it->second : nullptr;
        }

    private:
        std::unordered_map<RenderGraphResourceHandle, std::shared_ptr<Framebuffer>> &m_Resources;
    };

} // namespace Fermion
