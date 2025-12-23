#pragma once
#include "../Plugin.hpp"
namespace Fermion
{
    class BosonPlugin : public IPlugin
    {
    public:
        explicit BosonPlugin(const PluginSpecation &spec) : m_spec(spec)
        {
            m_spec.domain = PluginDomain::Editor;
        }
        virtual ~BosonPlugin() = default;
        virtual void onLoad() override;
        virtual void onUnload() override;
        virtual void onUIRender() override;

    private:
        PluginSpecation m_spec;
    };
}