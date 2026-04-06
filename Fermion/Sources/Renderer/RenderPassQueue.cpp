#include "RenderPassQueue.hpp"
#include "Renderer/RendererAPI.hpp"

namespace Fermion
{
    ResourceHandle RenderPassQueue::createResource()
    {
        ResourceHandle handle;
        m_Resources.push_back(handle);
        return handle;
    }

    RenderPassQueue::PassHandle RenderPassQueue::addPass(const RenderPass &pass)
    {
        m_Passes.push_back(pass);
        return m_Passes.size() - 1;
    }

    void RenderPassQueue::execute(RendererAPI &api)
    {
        for (const auto &pass : m_Passes)
        {
            if (pass.Execute)
                pass.Execute(api);
        }
    }

    void RenderPassQueue::reset()
    {
        m_Resources.clear();
        m_Passes.clear();
    }

} // namespace Fermion
