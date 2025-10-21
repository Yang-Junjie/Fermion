#pragma once
#include <SFML/System.hpp>
#include "Time/Timer.hpp"

namespace Fermion {

    class SFMLTimer : public ITimer
    {
    public:
        SFMLTimer() { reset(); }

        void reset() override
        {
            m_Clock.restart();
        }

        float elapsed() override
        {
            return m_Clock.getElapsedTime().asSeconds();
        }

        float elapsedMillis() override
        {
            return m_Clock.getElapsedTime().asMilliseconds();
        }

    private:
        sf::Clock m_Clock;
    };

}
