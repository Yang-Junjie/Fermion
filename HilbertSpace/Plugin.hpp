#pragma once
namespace Fermion
{
    enum class PluginDomain
    {
        None,
        Editor,
        Runtime
    };
    
    struct PluginSpecation
    {
        const char *name;
        PluginDomain domain;
        uint32_t version;
    };

    class IPlugin
    {
    public:
        virtual ~IPlugin() = default;

        virtual void onLoad() = 0;
        virtual void onUnload() = 0;

        virtual const PluginSpecation &getSpecation() const = 0;
    };

}