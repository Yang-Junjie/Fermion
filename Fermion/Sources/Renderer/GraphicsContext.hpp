
/*
    本文件抽象了图形上下文，并定义了创建图形上下文函数
*/
#pragma once
#include "fmpch.hpp"
namespace Fermion
{
    class GraphicsContext
    {
    public:
        virtual ~GraphicsContext() = default;
        
        virtual void init() = 0;
        virtual void swapBuffers() = 0;

        static std::unique_ptr<GraphicsContext> create(void *window);
    };
}