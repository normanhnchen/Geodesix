/**
 * ============================================================
 * Adapted from the official Vulkan Tutorial
 * https://docs.vulkan.org/tutorial/latest/00_Introduction.html
 * ============================================================
 */


#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <limits>
#include <map>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application.hpp"


void Application::Run() {
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}

void Application::InitWindow() {
    glfwInit();

    // Since GLFW creates an OpenGL context by default, we tell it to not create one
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    /**
     * !!!!!!!!!!!!!!!!!!!!!
     * NOTE: disable for now
     * !!!!!!!!!!!!!!!!!!!!!
     */
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Geodesix", nullptr, nullptr);
}

void Application::InitVulkan() {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    SelectPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
}

void Application::MainLoop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

void Application::Cleanup() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Application::CreateInstance() {
    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName = "Geodesix",
        .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .apiVersion = vk::ApiVersion14
    };

    // Get the required layers
    std::vector<char const*> requiredLayers;

#ifdef DEBUG_VALIDATION_LAYERS
    requiredLayers.assign(validationLayers.begin(), validationLayers.end());
#endif

    // Check if the required layers are supported by the Vulkan implementation.
    auto layerProperties = m_context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(
        requiredLayers,
        [&layerProperties](auto const &requiredLayer) {
            return std::ranges::none_of(
                layerProperties,
                [requiredLayer](auto const &layerProperty) {
                    // Validate the layer
                    return strcmp(layerProperty.layerName, requiredLayer) == 0;
                }
            );
        }
    );
    if (unsupportedLayerIt != requiredLayers.end()) {
        throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
    }

    std::vector<const char*> requiredExtensions = GetRequiredInstanceExtensions();

    // Check if the required extensions are supported by the Vulkan implementation
    auto extensions = m_context.enumerateInstanceExtensionProperties();
    
#ifdef DEBUG_PRINT_EXTENSIONS
    // Debug: print available extensions to the console
    std::cout << "Available extensions:\n";
    for (const auto& extension : extensions) {
        std::cout << "\t" << extension.extensionName << "\n";
    }
#endif
    
    // Verify all required extensions
    for (const char* requiredExtension : requiredExtensions) {
        if (std::ranges::none_of(
            extensions,
            [requiredExtension](auto const& extensionProperty) {
                // Validate the extension
                return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
            })
        ) {
            throw std::runtime_error(std::string("Required GLFW extension not supported: ") + requiredExtension);
        }
    }

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };

#ifdef __APPLE__
    // Add macOS / MoltenVK portability extension bit to prevent the possible error:
    // vk::Result::eErrorIncompatibleDriver
    createInfo.flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

    m_instance = vk::raii::Instance(m_context, createInfo);
}

std::vector<const char*> Application::GetRequiredInstanceExtensions() {
    // Get the required instance extensions from GLFW
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef __APPLE__
    // Add macOS / MoltenVK portability extension to prevent the possible error:
    // vk::Result::eErrorIncompatibleDriver
    requiredExtensions.push_back(vk::KHRPortabilityEnumerationExtensionName);
#endif

#ifdef DEBUG_VALIDATION_LAYERS
    // Add debug messenger (callback) for the validation layers
    requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
#endif

    return requiredExtensions;
}

void Application::SetupDebugMessenger() {
#ifndef DEBUG_VALIDATION_LAYERS
    return;
#endif

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
    );
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
    );
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
        .messageSeverity = severityFlags,
        .messageType = messageTypeFlags,
        .pfnUserCallback = &DebugCallback
    };
    m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(
        debugUtilsMessengerCreateInfoEXT
    );
}

void Application::SelectPhysicalDevice() {
    auto physicalDevices = m_instance.enumeratePhysicalDevices();
    if (physicalDevices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, vk::raii::PhysicalDevice> candidates;
    for (const auto& pd : physicalDevices) {
        if (!IsDeviceSuitable(pd)) {
            continue;
        }
        auto deviceProperties = pd.getProperties();
        auto deviceFeatures = pd.getFeatures();
        uint32_t score = 0;

        // Discrete GPUs are significantly more performant
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        }

        // Maximum possible size of textures
        score += deviceProperties.limits.maxImageDimension2D;

        candidates.insert(std::make_pair(score, pd));
    }

    // Check if the best candidate is suitable
    if (!candidates.empty()) {
        // Get the key of the last entry in the list (the best candidate)
        if (candidates.rbegin()->first > 0) {
            // Get the best candidate's value
            m_physicalDevice = candidates.rbegin()->second;
        }
    } else {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

bool Application::IsDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice) {
    // Check if the device supports Vulkan API version 1.3
    bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    bool supportsGraphics = std::ranges::any_of(
        queueFamilies, [](auto const &qfp) {
            return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
        }
    );

    // Check if each required device extension is supported by the physical device
    auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions = std::ranges::all_of(
        requiredDeviceExtension,
        [&availableDeviceExtensions](
            auto const & requiredDeviceExtension
        ) {
            return std::ranges::any_of(
                availableDeviceExtensions,
                [requiredDeviceExtension](
                    auto const & availableDeviceExtension
                ) {
                    return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0;
                }
            );
        }
    );

    // Check if the required features are supported by the physical device
    auto features = physicalDevice.template
        getFeatures2<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        >();
    bool supportsRequiredFeatures = (
        features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
        features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
        features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState
    );

    // Check if the physical device meets all of the required criteria
    if (
        supportsVulkan1_3 &&
        supportsGraphics &&
        supportsAllRequiredExtensions &&
        supportsRequiredFeatures
    ) {
        return true;
    } else {
        return false;
    }
}

void Application::CreateLogicalDevice() {
    // Get the first index of the queue with graphics capabilities
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

    // Bitwise operator; get the max possible uint32_t
    uint32_t queueIndex = ~0;

    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
        if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
            m_physicalDevice.getSurfaceSupportKHR(qfpIndex, *m_surface)) {
            // Found a queue family that supports both Vulkan graphics and present surfaces
            queueIndex = qfpIndex;
            break;
        }
    }
    if (queueIndex == ~0) {
        throw std::runtime_error("Could not find a queue for Vulkan graphics and present surfaces!");
    }

    // Decide which queues have relative priority over other queues
    float queuePriorities[] = {
        0.5f
    };

    vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
        .queueFamilyIndex = queueIndex,
        .queueCount = 1,
        .pQueuePriorities = queuePriorities
    };

    // Create a chain of feature structures
    vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
    >
    featureChain = {
        /* Physical device features */
        {
            // Empty for now
        },
        /* Vulkan 1.1 features */
        {
            .shaderDrawParameters = true
        },
        /* Vulkan 1.3 feature */
        {
            .dynamicRendering = true
        },
        /* Extended dynamic state features */
        {
            .extendedDynamicState = true
        }
    };

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
        .ppEnabledExtensionNames = requiredDeviceExtension.data()
    };

    m_device = vk::raii::Device(m_physicalDevice, deviceCreateInfo);
    m_graphicsQueue = vk::raii::Queue(m_device, queueIndex, 0);
}

void Application::CreateSurface() {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(*m_instance, m_window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create the GLFW-Vulkan window surface!");
    }

    m_surface = vk::raii::SurfaceKHR(m_instance, surface);
}

void Application::CreateSwapChain() {
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(*m_surface);
    m_swapChainExtent = ChooseSwapExtent(surfaceCapabilities);
    uint32_t minImageCount = ChooseSwapMinImageCount(surfaceCapabilities);
    
    std::vector<vk::SurfaceFormatKHR> availableFormats = m_physicalDevice.getSurfaceFormatsKHR(*m_surface);
    m_swapChainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

    std::vector<vk::PresentModeKHR> availablePresentModes = m_physicalDevice.getSurfacePresentModesKHR(*m_surface);
    vk::PresentModeKHR presentMode = ChooseSwapPresentMode(availablePresentModes);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{
        .surface = *m_surface,
        .minImageCount = minImageCount,
        .imageFormat = m_swapChainSurfaceFormat.format,
        .imageColorSpace = m_swapChainSurfaceFormat.colorSpace,
        .imageExtent = m_swapChainExtent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = true,
        /**
         * !!!!!!!!!!!!!!!!!!!!
         * NOTE: ignore for now
         * !!!!!!!!!!!!!!!!!!!!
         */
        .oldSwapchain = nullptr
    };

    m_swapChain = vk::raii::SwapchainKHR(m_device, swapChainCreateInfo);
    m_swapChainImages = m_swapChain.getImages();
}

vk::SurfaceFormatKHR Application::ChooseSwapSurfaceFormat(
    std::vector<vk::SurfaceFormatKHR> const &availableFormats
) {
    // Make sure there is an available format
    assert(!availableFormats.empty());

    // Checked if the preferred SRGB format is available
    const auto formatIt = std::ranges::find_if(
        availableFormats,
        [](const auto &format) {
            return (
                /* SRGB results in more accurate perceived colors */
                format.format == vk::Format::eB8G8R8A8Srgb &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear
            );
        }
    );

    if (formatIt != availableFormats.end()) {
        // Return the preferred available format
        return *formatIt;
    } else {
        // By default settle with the first available format
        return availableFormats[0];
    }
}

vk::PresentModeKHR Application::ChooseSwapPresentMode(
    std::vector<vk::PresentModeKHR> const &availablePresentModes
) {
    /**
     * - vk::PresentModeKHR::eImmediate
     *      Swap chain images are displayed immediately which may result in screen tearing.
     * - vk::PresentModeKHR::eFifo
     *      The swap chain is a queue of images where the screen displays images refreshed from the
     *      queue. When the queue is full, it blocks the application. This mode is similar to
     *      vertical sync.
     * - vk::PresentModeKHR::eFifoRelaxed
     *      This mode is the same as the vk::PresentModeKHR::eFifo except when waiting for the
     *      queue to fill, the image is displayed immediately which may result in screen tearing.
     * - vk::PresentModeKHR::eMailbox
     *      This is another variation of the vk::PresentModeKHR::eFifo except when the queue is
     *      full images already in the queue are replaced with newer ones. This mode is commonly
     *      known as triple buffering. This mode can be slightly more demanding to use than the
     *      FIFO mode.
     */

#ifdef DEBUG_PRESENT_IMMEDIATE
    return vk::PresentModeKHR::eImmediate;
#endif
#ifdef DEBUG_PRESENT_FIFO
    return vk::PresentModeKHR::eFifo;
#endif
#ifdef DEBUG_PRESENT_FIFO_RELAXED
    return vk::PresentModeKHR::eFifoRelaxed;
#endif
#ifdef DEBUG_PRESENT_MAILBOX
    return vk::PresentModeKHR::eMailbox;
#endif

    assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
    if (std::ranges::any_of(
        availablePresentModes,
        [](const vk::PresentModeKHR value) {
            return vk::PresentModeKHR::eMailbox == value;
        }
    )) {
        return vk::PresentModeKHR::eMailbox;
    } else {
        // Default to vk::PresentModeKHR::eFifo; guaranteed to be available
        return vk::PresentModeKHR::eFifo;
    }
}

vk::Extent2D Application::ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities) {
    // The window's extent is only std::numeric_limits<uint32_t>::max() if the
    // surface does not already want a fixed size
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        /* The surface already wants an exact size */
        return capabilities.currentExtent;
    } else {
        /* The surface has no size it specifically wants */

        int width, height;
        // Get the actual screen size in pixels
        glfwGetFramebufferSize(m_window, &width, &height);

        // Clamp to the surface's support range
        return {
            std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
    }
}

uint32_t Application::ChooseSwapMinImageCount(
    vk::SurfaceCapabilitiesKHR const &surfaceCapabilities
) {
    /**
     * Choose a specific minimum number of images to use in the swap chain.
     * 
     * The actual minimum number may cause the driver to wait before getting another image,
     * therefore the Vulkan tutorial recommends to request one more than the minimum. Also, the
     * minimum number of images must be less than the max amount supported by the surface.
     */

#ifdef DEBUG_MIN_IMAGE_COUNT_LEGACY
    /* Tutorial's explanatory version */

    uint32_t minImageCount = surfaceCapabilities.minImageCount + 1u;
#else
    /* Tutorial's actual shipped version */
    
    uint32_t minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
#endif

    // NOTE: Vulkan's maxImageCount == 0 means that there is no upper limit
    if (
        (0 < surfaceCapabilities.maxImageCount) &&
        (minImageCount > surfaceCapabilities.maxImageCount)
    ) {
        minImageCount = surfaceCapabilities.maxImageCount;
    }

    return minImageCount;
}
