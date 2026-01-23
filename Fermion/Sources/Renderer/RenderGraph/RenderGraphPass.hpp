#pragma once
#include "PassContext.hpp"
#include "RenderGraphResource.hpp"
#include <functional>
#include <string>
#include <vector>

namespace Fermion
{
    class RenderGraphPass
    {
    public:
        using ExecuteFunc = std::function<void(PassContext &)>;
        using ConditionFunc = std::function<bool()>;

        RenderGraphPass(const std::string &name) : m_Name(name) {}

        const std::string &getName() const { return m_Name; }
        const std::vector<RenderGraphResourceHandle> &getInputs() const { return m_Inputs; }
        const std::vector<RenderGraphResourceHandle> &getOutputs() const { return m_Outputs; }
        const ExecuteFunc &getExecuteFunc() const { return m_ExecuteFunc; }
        const ConditionFunc &getConditionFunc() const { return m_ConditionFunc; }

        bool shouldExecute() const { return !m_ConditionFunc || m_ConditionFunc(); }

    private:
        friend class PassBuilder;

        std::string m_Name;
        std::vector<RenderGraphResourceHandle> m_Inputs;
        std::vector<RenderGraphResourceHandle> m_Outputs;
        ExecuteFunc m_ExecuteFunc;
        ConditionFunc m_ConditionFunc;
    };

    class PassBuilder
    {
    public:
        PassBuilder(const std::string &name) : m_Pass(name) {}

        PassBuilder &read(RenderGraphResourceHandle resource)
        {
            m_Pass.m_Inputs.push_back(resource);
            return *this;
        }

        PassBuilder &write(RenderGraphResourceHandle resource)
        {
            m_Pass.m_Outputs.push_back(resource);
            return *this;
        }

        PassBuilder &condition(RenderGraphPass::ConditionFunc func)
        {
            m_Pass.m_ConditionFunc = std::move(func);
            return *this;
        }

        PassBuilder &execute(RenderGraphPass::ExecuteFunc func)
        {
            m_Pass.m_ExecuteFunc = std::move(func);
            return *this;
        }

        RenderGraphPass build() { return std::move(m_Pass); }

    private:
        RenderGraphPass m_Pass;
    };

} // namespace Fermion
