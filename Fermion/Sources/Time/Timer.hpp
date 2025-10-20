#pragma once

namespace Fermion {

    class ITimer
    {
    public:
        virtual ~ITimer() = default;

        virtual void Reset() = 0;
        virtual float Elapsed() = 0;        // 返回秒
        virtual float ElapsedMillis() = 0;  // 返回毫秒
    };

}
