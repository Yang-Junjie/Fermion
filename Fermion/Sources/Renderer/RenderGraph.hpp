#pragma once
namespace Fermion
{
    class RenderGraph
    {
    public:
        void AddPass(const RenderPass &pass)
        {
            m_Passes.push_back(pass);
        }

        void Execute()
        {
            for (auto &pass : m_Passes)
                pass.Execute();
        }

        void Reset()
        {
            m_Passes.clear();
        }

    private:
        std::vector<RenderPass> m_Passes;
    };

}