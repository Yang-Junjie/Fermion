#pragma once
#include "Core/Window.h"
#include <SFML/Graphics.hpp>
#include "Events/ApplicationEvent.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"

namespace Oxygine
{
    class SFMLWindow : public IWindow
    {
    public:
        SFMLWindow();
        SFMLWindow(const WindowProps &props);
        virtual ~SFMLWindow() = default;

        bool isOpen() const override;
        void pollEvents() override;
        void clear() override;
        void display() override;

        virtual uint32_t getWidth() const override { return m_data.width; }
        virtual uint32_t getHeight() const override { return m_data.height; }

        virtual void setEventCallback(const EventCallbackFn &callback) override { m_data.EventCallback = callback; }

        sf::RenderWindow &get();

        virtual void OnUpdate() override;

    private:
        virtual void Init(const WindowProps &props);
        void processEvent(const sf::Event &event);

    private:
        struct WindowData
        {
            std::string title;
            unsigned int width, height;
            bool VSync;
            EventCallbackFn EventCallback;
        };

        sf::RenderWindow m_window;
        WindowData m_data;
    };
}