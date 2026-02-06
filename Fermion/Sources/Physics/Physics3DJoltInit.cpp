#include "fmpch.hpp"
#include "Physics/Physics3DJoltInit.hpp"
#include "Core/Log.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>

#include <cstdarg>
#include <cstdlib>
#include <format>

namespace
{
    bool s_joltInitialized = false;

    void TraceImpl(const char *inFMT, ...)
    {
        char buffer[1024];
        va_list list;
        va_start(list, inFMT);
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);
        Fermion::Log::Debug(std::format("[Jolt] {}", buffer));
    }

#ifdef JPH_ENABLE_ASSERTS
    bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, JPH::uint inLine)
    {
        Fermion::Log::Error(std::format("Jolt assert failed: {} ({}:{}): {}", inExpression ? inExpression : "",
                                        inFile ? inFile : "?", inLine, inMessage ? inMessage : ""));
        return true;
    }
#endif

    void ShutdownJolt()
    {
        if (!s_joltInitialized)
            return;

        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
        s_joltInitialized = false;
    }

} // anonymous namespace

namespace Fermion::Physics3DInternal
{
    void InitializeJoltIfNeeded()
    {
        if (s_joltInitialized)
            return;

        JPH::RegisterDefaultAllocator();
        JPH::Trace = TraceImpl;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        std::atexit(ShutdownJolt);

        s_joltInitialized = true;
    }

} // namespace Fermion::Physics3DInternal
