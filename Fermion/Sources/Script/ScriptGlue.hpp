#pragma once
#include "mono/metadata/object.h"
namespace Fermion
{

    class ScriptGlue
    {
    public:
        static void registerComponents();
        static void registerComponentFactories();
        static void registerFunctions();
    };

}
