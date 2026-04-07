#include "Core/KeyCodes.hpp"
#include "Core/Log.hpp"
#include "Core/Window.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Events/Event.hpp"
#include "Events/KeyEvent.hpp"
#include "Renderer/RendererAPI.hpp"

#include <chrono>
#include <optional>
#include <string_view>

namespace {

bool HandleWindowClose(Fermion::WindowCloseEvent&, bool& running)
{
    running = false;
    return true;
}

bool HandleKeyPressed(Fermion::KeyPressedEvent& event, bool& running)
{
    if (event.getKeyCode() != Fermion::KeyCode::Escape) {
        return false;
    }

    running = false;
    return true;
}

std::optional<double> ParseDurationSeconds(int argc, char** argv)
{
    constexpr std::string_view prefix = "--duration=";

    for (int index = 1; index < argc; ++index) {
        const std::string_view argument = argv[index];
        if (!argument.starts_with(prefix)) {
            continue;
        }

        const std::string_view value = argument.substr(prefix.size());
        if (value.empty()) {
            return std::nullopt;
        }

        try {
            return std::stod(std::string(value));
        } catch (...) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

} // namespace

int main(int argc, char** argv)
{
    Fermion::Log::Init("vulkan-frame-loop.log", Fermion::LogLevel::Debug);
    Fermion::RendererAPI::setAPI(Fermion::RendererAPI::API::Vulkan);
    const std::optional<double> durationSeconds = ParseDurationSeconds(argc, argv);

    Fermion::WindowProps props("Vulkan Frame Loop", 1280, 720);
    auto window = Fermion::IWindow::create(props);
    if (!window) {
        Fermion::Log::Critical("Failed to create VulkanFrameLoop window.");
        return 1;
    }

    if (!window->getGraphicsContext()) {
        Fermion::Log::Critical("VulkanFrameLoop did not receive a graphics context.");
        return 1;
    }

    const Fermion::DeviceInfo deviceInfo = window->getDeviceInfo();
    Fermion::Log::Info(std::format("VulkanFrameLoop device: {} / {} / {}",
                                   deviceInfo.vendor,
                                   deviceInfo.renderer,
                                   deviceInfo.version));
    if (durationSeconds.has_value()) {
        Fermion::Log::Info(
            std::format("VulkanFrameLoop auto-exit after {:.2f} seconds", *durationSeconds));
    }

    bool running = true;
    const auto startTime = std::chrono::steady_clock::now();
    window->setEventCallback([&running](Fermion::IEvent& event) {
        Fermion::EventDispatcher dispatcher(event);

        dispatcher.dispatch<Fermion::WindowCloseEvent>(
            [&running](Fermion::WindowCloseEvent& closeEvent) {
                return HandleWindowClose(closeEvent, running);
            });

        dispatcher.dispatch<Fermion::KeyPressedEvent>(
            [&running](Fermion::KeyPressedEvent& keyEvent) {
                return HandleKeyPressed(keyEvent, running);
            });

        dispatcher.dispatch<Fermion::WindowResizeEvent>(
            [](Fermion::WindowResizeEvent& resizeEvent) {
                Fermion::Log::Info(
                    std::format("VulkanFrameLoop resized to {}x{}",
                                resizeEvent.getWidth(),
                                resizeEvent.getHeight()));
                return false;
            });
    });

    while (running) {
        if (durationSeconds.has_value()) {
            const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                                               startTime);
            if (elapsed.count() >= *durationSeconds) {
                running = false;
                continue;
            }
        }

        window->onUpdate();
    }

    return 0;
}
