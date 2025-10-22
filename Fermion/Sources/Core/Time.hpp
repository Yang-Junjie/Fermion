#pragma once
#include "fmpch.hpp"
#include "Time/Timer.hpp"

namespace Fermion {

    class ChronoTimer : public ITimer
    {
    public:
        ChronoTimer() { reset(); }

        void reset() override
        {
            m_Start = std::chrono::high_resolution_clock::now();
        }

        float elapsed() override
        {
            auto now = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<float>(now - m_Start).count();
        }

        float elapsedMillis() override
        {
            return elapsed() * 1000.0f;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
    };

}
