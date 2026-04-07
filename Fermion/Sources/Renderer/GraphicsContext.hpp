#pragma once
#include "fmpch.hpp"
namespace Fermion
{
    struct DeviceInfo
    {
        std::string renderer;
        std::string vendor;
        std::string version;
    };
    class GraphicsContext
    {
    public:
        virtual ~GraphicsContext() = default;

        virtual void init() = 0;
        virtual void present() = 0;
        virtual void resize(uint32_t width, uint32_t height) {}
        virtual void setVSync(bool enabled) {}

        static std::unique_ptr<GraphicsContext> create(void *window);

        const DeviceInfo &getDeviceInfo() const
        {
            return m_deviceInfo;
        }

    protected:
        DeviceInfo m_deviceInfo;
    };
} // namespace Fermion
