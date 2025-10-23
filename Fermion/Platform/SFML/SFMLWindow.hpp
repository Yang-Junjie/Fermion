#pragma once
#include "Core/Window.hpp"
#include <SFML/Graphics.hpp>
#include "Events/ApplicationEvent.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"
#include <unordered_set>

namespace Fermion
{
    class SFMLWindow : public IWindow
    {
    public:
        SFMLWindow();
        SFMLWindow(const WindowProps &props,sf::ContextSettings settings = {0,0,0,3,3,sf::ContextSettings::Core});
        ~SFMLWindow();

        virtual uint32_t getWidth() const override { return m_data.width; }
        virtual uint32_t getHeight() const override { return m_data.height; }

        virtual void setEventCallback(const EventCallbackFn &callback) override { m_data.EventCallback = callback; }

        virtual bool isOpen() const override;
        virtual void pollEvents() override;
        virtual void clear() override;
        virtual void display() override;
        virtual void setVSync(bool enabled) override;
        virtual bool isVSync() const override;
        virtual void OnUpdate() override;
        sf::RenderWindow &getWindow();

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
        std::unordered_set<sf::Keyboard::Key> m_heldKeys;
        sf::Clock m_deltaClock;
    };
}