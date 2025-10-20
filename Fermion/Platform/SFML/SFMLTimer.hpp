#pragma once
#include <SFML/System.hpp>
#include "Time/Timer.hpp"

namespace Fermion {

    class SFMLTimer : public ITimer
    {
    public:
        SFMLTimer() { Reset(); }

        void Reset() override
        {
            m_Clock.restart();
        }

        float Elapsed() override
        {
            return m_Clock.getElapsedTime().asSeconds();
        }

        float ElapsedMillis() override
        {
            return m_Clock.getElapsedTime().asMilliseconds();
        }

    private:
        sf::Clock m_Clock;
    };

}
