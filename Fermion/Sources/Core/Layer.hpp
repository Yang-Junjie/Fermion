/*
    Layer.hpp
    本文件定义了抽象层的基类，层是引擎中可被添加的模块，每个游戏逻辑模块都是一个层。
    如：渲染层，逻辑层，UI层等等。
    抽象层类定义了层中必须实现的方法)。
    onAttach()方法在层被添加到引擎中时调用
    onDetach()方法在层被解除时调用
    onUpdate()方法会在每一帧调用
    onEvent()方法会在每一帧的每一帧调用
    onImGuiRender()方法会在每一帧的ImGui渲染时调用。
*/
#pragma once
#include "Events/Event.hpp"
#include "Core/Timestep.hpp"
namespace Fermion
{
    class Layer
    {
    public:
        Layer(const std::string &name = "Layer") : m_name(name) {}
        virtual ~Layer() = default;

        virtual void onAttach() {};
        virtual void onDetach() {};
        virtual void onUpdate(Timestep dt) {};
        virtual void onEvent(IEvent &event) {};
        virtual void onImGuiRender() {};

        const std::string &getName() const { return m_name; }

    protected:
        std::string m_name;
        // Layer比Renderer的生命周期更长，所以Layer中可以持有Renderer的指针
    };
}