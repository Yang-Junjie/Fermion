#include "Core/Log.hpp"
#include "fmpch.hpp"
#include "VulkanContext.hpp"

#include <array>
#include <algorithm>
#include <limits>
#include <set>
#include <vector>

namespace Fermion {
namespace {

constexpr std::array<const char*, 1> kRequiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

std::string VendorName(uint32_t vendorId)
{
    switch (vendorId) {
        case 0x10'02:
        case 0x10'22:
            return "AMD";
        case 0x10'DE:
            return "NVIDIA";
        case 0x13'B5:
            return "ARM";
        case 0x51'43:
            return "Qualcomm";
        case 0x80'86:
            return "Intel";
        case 0x10'6B:
            return "Apple";
        default:
            return std::format("Vendor 0x{:04X}", vendorId);
    }
}

std::string VersionString(uint32_t version)
{
    return std::format("{}.{}.{}",
                       VK_VERSION_MAJOR(version),
                       VK_VERSION_MINOR(version),
                       VK_VERSION_PATCH(version));
}

const char* VkResultString(VkResult result)
{
    switch (result) {
        case VK_SUCCESS:
            return "VK_SUCCESS";
        case VK_NOT_READY:
            return "VK_NOT_READY";
        case VK_TIMEOUT:
            return "VK_TIMEOUT";
        case VK_EVENT_SET:
            return "VK_EVENT_SET";
        case VK_EVENT_RESET:
            return "VK_EVENT_RESET";
        case VK_INCOMPLETE:
            return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_SUBOPTIMAL_KHR:
            return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "VK_ERROR_SURFACE_LOST_KHR";
        default:
            return "VK_ERROR_UNKNOWN";
    }
}

} // namespace

VulkanContext::VulkanContext(void* windowHandle)
    : m_windowHandle(static_cast<GLFWwindow*>(windowHandle))
{
    Log::Info("Vulkan context bootstrap created.");
}

VulkanContext::~VulkanContext()
{
    destroy();
}

void VulkanContext::init()
{
    FM_PROFILE_FUNCTION();

    if (!m_windowHandle) {
        Log::Error("VulkanContext requires a valid native window.");
        return;
    }

    if (glfwVulkanSupported() != GLFW_TRUE) {
        Log::Error("GLFW reports that Vulkan is not supported on this machine.");
        return;
    }

    createInstance();
    if (!m_instance) {
        return;
    }

    createSurface();
    if (!m_surface) {
        destroy();
        return;
    }

    pickPhysicalDevice();
    if (!m_physicalDevice) {
        destroy();
        return;
    }

    createLogicalDevice();
    if (!m_device) {
        destroy();
        return;
    }

    createCommandPool();
    if (!m_commandPool) {
        destroy();
        return;
    }

    createCommandBuffer();
    if (!m_commandBuffer) {
        destroy();
        return;
    }

    createSyncObjects();
    if (!m_imageAvailableSemaphore || !m_renderFinishedSemaphore || !m_inFlightFence) {
        destroy();
        return;
    }

    if (!createSwapchain()) {
        destroy();
        return;
    }

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

    m_deviceInfo.renderer = properties.deviceName;
    m_deviceInfo.vendor = VendorName(properties.vendorID);
    m_deviceInfo.version = VersionString(properties.apiVersion);

    Log::Info(std::format("Vulkan bootstrap ready: {} / {} / API {}",
                          m_deviceInfo.vendor,
                          m_deviceInfo.renderer,
                          m_deviceInfo.version));
}

void VulkanContext::present()
{
    if (!m_device || !m_swapchain) {
        return;
    }

    if (m_resizePending && !recreateSwapchain()) {
        return;
    }

    VkResult waitResult = vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    if (waitResult != VK_SUCCESS) {
        Log::Error(std::format("Failed waiting for Vulkan frame fence: {}",
                               VkResultString(waitResult)));
        return;
    }

    uint32_t imageIndex = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(m_device,
                                                   m_swapchain,
                                                   UINT64_MAX,
                                                   m_imageAvailableSemaphore,
                                                   VK_NULL_HANDLE,
                                                   &imageIndex);
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        m_resizePending = true;
        recreateSwapchain();
        return;
    }

    if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
        Log::Error(std::format("Failed to acquire Vulkan swapchain image: {}",
                               VkResultString(acquireResult)));
        return;
    }

    VkResult resetFenceResult = vkResetFences(m_device, 1, &m_inFlightFence);
    if (resetFenceResult != VK_SUCCESS) {
        Log::Error(std::format("Failed to reset Vulkan frame fence: {}",
                               VkResultString(resetFenceResult)));
        return;
    }

    VkResult resetCommandBufferResult = vkResetCommandBuffer(m_commandBuffer, 0);
    if (resetCommandBufferResult != VK_SUCCESS) {
        Log::Error(std::format("Failed to reset Vulkan command buffer: {}",
                               VkResultString(resetCommandBufferResult)));
        return;
    }

    if (!recordClearCommandBuffer(imageIndex)) {
        return;
    }

    const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderFinishedSemaphore;

    VkResult submitResult = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFence);
    if (submitResult != VK_SUCCESS) {
        Log::Error(std::format("Failed to submit Vulkan command buffer: {}",
                               VkResultString(submitResult)));
        return;
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &imageIndex;

    VkResult presentResult = vkQueuePresentKHR(m_presentQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR ||
        m_resizePending) {
        recreateSwapchain();
        return;
    }

    if (presentResult != VK_SUCCESS) {
        Log::Error(
            std::format("Failed to present Vulkan swapchain image: {}",
                        VkResultString(presentResult)));
    }
}

void VulkanContext::resize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0) {
        return;
    }

    m_resizePending = true;
}

void VulkanContext::setVSync(bool enabled)
{
    if (m_vsyncEnabled == enabled) {
        return;
    }

    m_vsyncEnabled = enabled;
    m_resizePending = true;
}

void VulkanContext::createInstance()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (!glfwExtensions || glfwExtensionCount == 0) {
        Log::Error("Failed to query required GLFW Vulkan instance extensions.");
        return;
    }

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Fermion";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.pEngineName = "Fermion";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        Log::Error(std::format("Failed to create Vulkan instance: {}", VkResultString(result)));
        m_instance = VK_NULL_HANDLE;
    }
}

void VulkanContext::createSurface()
{
    VkResult result = glfwCreateWindowSurface(m_instance, m_windowHandle, nullptr, &m_surface);
    if (result != VK_SUCCESS) {
        Log::Error(std::format("Failed to create Vulkan surface: {}", VkResultString(result)));
        m_surface = VK_NULL_HANDLE;
    }
}

void VulkanContext::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        Log::Error("No Vulkan physical devices were found.");
        return;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    VkPhysicalDevice fallbackDevice = VK_NULL_HANDLE;
    for (VkPhysicalDevice device : devices) {
        if (!supportsRequiredDeviceExtensions(device)) {
            continue;
        }

        QueueFamilyIndices indices = findQueueFamilies(device);
        if (!indices.isComplete()) {
            continue;
        }

        const SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
        if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty()) {
            continue;
        }

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            m_physicalDevice = device;
            return;
        }

        if (fallbackDevice == VK_NULL_HANDLE) {
            fallbackDevice = device;
        }
    }

    m_physicalDevice = fallbackDevice;
    if (m_physicalDevice == VK_NULL_HANDLE) {
        Log::Error("Failed to find a Vulkan physical device with graphics, present and swapchain "
                   "support.");
    }
}

void VulkanContext::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
    if (!indices.isComplete()) {
        Log::Error("The selected Vulkan physical device does not expose complete queue families.");
        return;
    }

    std::set<uint32_t> uniqueQueueFamilies = {*indices.graphicsFamily, *indices.presentFamily};

    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueQueueFamilies.size());

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(kRequiredDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = kRequiredDeviceExtensions.data();

    VkResult result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
    if (result != VK_SUCCESS) {
        Log::Error(
            std::format("Failed to create Vulkan logical device: {}", VkResultString(result)));
        m_device = VK_NULL_HANDLE;
        return;
    }

    vkGetDeviceQueue(m_device, *indices.graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, *indices.presentFamily, 0, &m_presentQueue);
    m_graphicsQueueFamily = *indices.graphicsFamily;
    m_presentQueueFamily = *indices.presentFamily;
}

bool VulkanContext::createSwapchain()
{
    const SwapchainSupportDetails support = querySwapchainSupport(m_physicalDevice);
    if (support.formats.empty() || support.presentModes.empty()) {
        Log::Error("Failed to create Vulkan swapchain: surface formats or present modes are empty.");
        return false;
    }

    if ((support.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
        Log::Error("The Vulkan surface does not support VK_IMAGE_USAGE_TRANSFER_DST_BIT for swapchain images.");
        return false;
    }

    const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(support.formats);
    const VkPresentModeKHR presentMode = choosePresentMode(support.presentModes);
    const VkExtent2D extent = chooseSwapExtent(support.capabilities);
    if (extent.width == 0 || extent.height == 0) {
        return false;
    }

    uint32_t imageCount = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0 &&
        imageCount > support.capabilities.maxImageCount) {
        imageCount = support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.preTransform = support.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    const uint32_t queueFamilyIndices[] = {m_graphicsQueueFamily, m_presentQueueFamily};
    if (m_graphicsQueueFamily != m_presentQueueFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkResult result = vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain);
    if (result != VK_SUCCESS) {
        Log::Error(std::format("Failed to create Vulkan swapchain: {}",
                               VkResultString(result)));
        m_swapchain = VK_NULL_HANDLE;
        return false;
    }

    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());
    m_swapchainImageInitialized.assign(imageCount, false);
    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;
    m_resizePending = false;

    Log::Info(std::format("Vulkan swapchain ready: {} images, {}x{}, present mode {}",
                          imageCount,
                          m_swapchainExtent.width,
                          m_swapchainExtent.height,
                          presentMode == VK_PRESENT_MODE_FIFO_KHR ? "FIFO"
                                                                  : "MAILBOX"));
    return true;
}

void VulkanContext::destroySwapchain()
{
    m_swapchainImageInitialized.clear();
    m_swapchainImages.clear();

    if (m_swapchain) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }

    m_swapchainImageFormat = VK_FORMAT_UNDEFINED;
    m_swapchainExtent = {0, 0};
}

bool VulkanContext::recreateSwapchain()
{
    if (!m_device || !m_surface || !m_windowHandle) {
        return false;
    }

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(m_windowHandle, &framebufferWidth, &framebufferHeight);
    if (framebufferWidth == 0 || framebufferHeight == 0) {
        return false;
    }

    VkResult waitIdleResult = vkDeviceWaitIdle(m_device);
    if (waitIdleResult != VK_SUCCESS) {
        Log::Error(std::format("Failed waiting for Vulkan device idle during swapchain recreation: {}",
                               VkResultString(waitIdleResult)));
        return false;
    }

    destroySwapchain();
    return createSwapchain();
}

void VulkanContext::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_graphicsQueueFamily;

    VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
    if (result != VK_SUCCESS) {
        Log::Error(std::format("Failed to create Vulkan command pool: {}",
                               VkResultString(result)));
        m_commandPool = VK_NULL_HANDLE;
    }
}

void VulkanContext::createCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer);
    if (result != VK_SUCCESS) {
        Log::Error(std::format("Failed to allocate Vulkan command buffer: {}",
                               VkResultString(result)));
        m_commandBuffer = VK_NULL_HANDLE;
    }
}

void VulkanContext::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult imageAvailableResult =
        vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore);
    VkResult renderFinishedResult =
        vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore);
    VkResult fenceResult = vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFence);

    if (imageAvailableResult != VK_SUCCESS || renderFinishedResult != VK_SUCCESS ||
        fenceResult != VK_SUCCESS) {
        Log::Error(std::format(
            "Failed to create Vulkan sync objects: imageAvailable={}, renderFinished={}, fence={}",
            VkResultString(imageAvailableResult),
            VkResultString(renderFinishedResult),
            VkResultString(fenceResult)));
    }
}

void VulkanContext::destroy()
{
    if (m_device) {
        vkDeviceWaitIdle(m_device);
    }

    destroySwapchain();

    if (m_commandPool && m_commandBuffer) {
        vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandBuffer);
        m_commandBuffer = VK_NULL_HANDLE;
    }

    if (m_commandPool) {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }

    if (m_imageAvailableSemaphore) {
        vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr);
        m_imageAvailableSemaphore = VK_NULL_HANDLE;
    }

    if (m_renderFinishedSemaphore) {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphore, nullptr);
        m_renderFinishedSemaphore = VK_NULL_HANDLE;
    }

    if (m_inFlightFence) {
        vkDestroyFence(m_device, m_inFlightFence, nullptr);
        m_inFlightFence = VK_NULL_HANDLE;
    }

    if (m_device) {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    if (m_surface && m_instance) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    if (m_instance) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }

    m_physicalDevice = VK_NULL_HANDLE;
    m_graphicsQueue = VK_NULL_HANDLE;
    m_presentQueue = VK_NULL_HANDLE;
    m_graphicsQueueFamily = 0;
    m_presentQueueFamily = 0;
}

bool VulkanContext::recordClearCommandBuffer(uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult beginResult = vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    if (beginResult != VK_SUCCESS) {
        Log::Error(std::format("Failed to begin Vulkan command buffer: {}",
                               VkResultString(beginResult)));
        return false;
    }

    VkImageMemoryBarrier toTransferBarrier{};
    toTransferBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toTransferBarrier.oldLayout = m_swapchainImageInitialized[imageIndex]
                                      ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                                      : VK_IMAGE_LAYOUT_UNDEFINED;
    toTransferBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toTransferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransferBarrier.image = m_swapchainImages[imageIndex];
    toTransferBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toTransferBarrier.subresourceRange.baseMipLevel = 0;
    toTransferBarrier.subresourceRange.levelCount = 1;
    toTransferBarrier.subresourceRange.baseArrayLayer = 0;
    toTransferBarrier.subresourceRange.layerCount = 1;
    toTransferBarrier.srcAccessMask = 0;
    toTransferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    const VkPipelineStageFlags srcStage = m_swapchainImageInitialized[imageIndex]
                                              ? VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
                                              : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    const VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    vkCmdPipelineBarrier(m_commandBuffer,
                         srcStage,
                         dstStage,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &toTransferBarrier);

    const VkClearColorValue clearColor = {{0.08f, 0.09f, 0.12f, 1.0f}};
    VkImageSubresourceRange clearRange{};
    clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearRange.baseMipLevel = 0;
    clearRange.levelCount = 1;
    clearRange.baseArrayLayer = 0;
    clearRange.layerCount = 1;
    vkCmdClearColorImage(m_commandBuffer,
                         m_swapchainImages[imageIndex],
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         &clearColor,
                         1,
                         &clearRange);

    VkImageMemoryBarrier toPresentBarrier{};
    toPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    toPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toPresentBarrier.image = m_swapchainImages[imageIndex];
    toPresentBarrier.subresourceRange = clearRange;
    toPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    toPresentBarrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(m_commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &toPresentBarrier);

    VkResult endResult = vkEndCommandBuffer(m_commandBuffer);
    if (endResult != VK_SUCCESS) {
        Log::Error(std::format("Failed to end Vulkan command buffer: {}",
                               VkResultString(endResult)));
        return false;
    }

    m_swapchainImageInitialized[imageIndex] = true;
    return true;
}

VulkanContext::QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice device) const
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
        if (presentSupport == VK_TRUE) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}

bool VulkanContext::supportsRequiredDeviceExtensions(VkPhysicalDevice device) const
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(
        device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(kRequiredDeviceExtensions.begin(),
                                             kRequiredDeviceExtensions.end());
    for (const VkExtensionProperties& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VulkanContext::SwapchainSupportDetails VulkanContext::querySwapchainSupport(
    VkPhysicalDevice device) const
{
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
    if (formatCount > 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device,
                                             m_surface,
                                             &formatCount,
                                             details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
    if (presentModeCount > 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                                  m_surface,
                                                  &presentModeCount,
                                                  details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR VulkanContext::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) const
{
    for (const VkSurfaceFormatKHR& format : availableFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return availableFormats.front();
}

VkPresentModeKHR VulkanContext::choosePresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes) const
{
    if (!m_vsyncEnabled) {
        for (const VkPresentModeKHR mode : availablePresentModes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return mode;
            }
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(m_windowHandle, &framebufferWidth, &framebufferHeight);
    if (framebufferWidth <= 0 || framebufferHeight <= 0) {
        return {0, 0};
    }

    VkExtent2D actualExtent = {static_cast<uint32_t>(framebufferWidth),
                               static_cast<uint32_t>(framebufferHeight)};
    actualExtent.width = std::clamp(actualExtent.width,
                                    capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height,
                                     capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);
    return actualExtent;
}

} // namespace Fermion
