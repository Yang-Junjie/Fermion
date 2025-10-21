#pragma once

namespace Fermion {

    class ITimer
    {
    public:
        virtual ~ITimer() = default;

        virtual void reset() = 0;
        virtual float elapsed() = 0;        // 返回秒
        virtual float elapsedMillis() = 0;  // 返回毫秒
    };

}
