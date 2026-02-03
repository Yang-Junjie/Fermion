#include "RenderGraphLegacy.hpp"

namespace Fermion
{
    ResourceHandle RenderGraphLegacy::createResource()
    {
        RenderGraphResourceDesc desc;
        desc.isTransient = true;
        desc.width = 1920;
        desc.height = 1080;
        desc.format = FramebufferTextureFormat::RGBA8;

        return m_Graph.declareResource("LegacyResource", desc);
    }

    RenderGraphLegacy::PassHandle RenderGraphLegacy::addPass(const LegacyRenderGraphPass &pass)
    {
        m_LegacyPasses.push_back(pass);

        m_Graph.addPass(pass.Name, [pass](PassBuilder &builder)
                        {
            for (const auto &input : pass.Inputs)
            {
                if (input.isValid())
                    builder.read(input);
            }

            for (const auto &output : pass.Outputs)
            {
                if (output.isValid())
                    builder.write(output);
            }

            builder.execute([pass](PassContext &ctx)
                            {
                if (pass.Execute)
                    pass.Execute(ctx.commandQueue);
            });
        });

        return m_LegacyPasses.size() - 1;
    }

    bool RenderGraphLegacy::compile()
    {
        return m_Graph.compile();
    }

    void RenderGraphLegacy::execute(RenderCommandQueue &queue, RendererAPI &api)
    {
        m_Graph.execute(queue, api);
    }

    void RenderGraphLegacy::reset()
    {
        m_Graph.reset();
        m_LegacyPasses.clear();
    }

    bool RenderGraphLegacy::lastCompileSucceeded() const
    {
        return m_Graph.lastCompileSucceeded();
    }

} // namespace Fermion
