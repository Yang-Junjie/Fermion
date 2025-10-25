/*
    本文化定义了渲染API
*/

#pragma once
#include "glm.hpp"
#include "fmpch.hpp"
namespace Fermion
{

    class RendererAPI
    {
    public:
        enum class API : int8_t
        {
            None = 0,
            OpenGL = 1
        };

    public:
        virtual ~RendererAPI() = default;

        virtual void init() = 0;
        virtual void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
        virtual void setClearColor(const glm::vec4 &color) = 0;
        virtual void clear() = 0;

        static API getAPI() { return s_API; }
        static std::unique_ptr<RendererAPI> create();

    private:
        static API s_API;
    };

}
