#pragma once
#include "Core/UUID.hpp"
#include "Renderer/Framebuffer.hpp"
#include <memory>
#include <string>

namespace Fermion
{
    using RenderGraphResourceHandle = UUID;

    enum class RenderGraphResourceType
    {
        Texture2D,
        TextureCube,
        Buffer
    };

    struct RenderGraphResourceDesc
    {
        RenderGraphResourceType type = RenderGraphResourceType::Texture2D;
        uint32_t width = 0;
        uint32_t height = 0;
        FramebufferTextureFormat format = FramebufferTextureFormat::RGBA8;
        bool isTransient = true;
    };

    class RenderGraphResource
    {
    public:
        RenderGraphResource(const std::string &name, const RenderGraphResourceDesc &desc);
        RenderGraphResource(const std::string &name, std::shared_ptr<Framebuffer> external);

        const std::string &getName() const { return m_Name; }
        RenderGraphResourceHandle getHandle() const { return m_Handle; }
        const RenderGraphResourceDesc &getDesc() const { return m_Desc; }
        bool isExternal() const { return m_IsExternal; }
        bool isTransient() const { return m_Desc.isTransient; }

        void setFramebuffer(std::shared_ptr<Framebuffer> framebuffer) { m_Framebuffer = framebuffer; }
        std::shared_ptr<Framebuffer> getFramebuffer() const { return m_Framebuffer; }

    private:
        std::string m_Name;
        RenderGraphResourceHandle m_Handle;
        RenderGraphResourceDesc m_Desc;
        bool m_IsExternal = false;
        std::shared_ptr<Framebuffer> m_Framebuffer;
    };

} // namespace Fermion
