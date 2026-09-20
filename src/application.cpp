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


/**
 * @brief Runs the application.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
void Application::Run() {
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}

/**
 * @brief Initializes the GLFW window.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
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

/**
 * @brief Initializes the Vulkan library and calls all of the required helper initialization
 * functions in the *required dependency order*:
 * 
 * (Instance -> (Validation Layers & Debug messenger) -> Surface -> Physical Device -> Logical
 *  Device -> Swap Chain -> Image Views)
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
void Application::InitVulkan() {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    SelectPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateCommandBuffer();
    CreateSyncObjects();
}

/**
 * @brief Main rendering loop.
 * 
 * TODO: currently only polls GLFW events; add actual rendering implementations and Vulkan API
 * calls.
 */
void Application::MainLoop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        DrawFrame();
    }

    // Wait for the logical device to finish its operations before terminating
    m_device.waitIdle();
}

/**
 * @brief Destroys the window and terminates GLFW before terminating the program. Vulkan resources
 * are cleaned up via RAII.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
void Application::Cleanup() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

/**
 * @brief Initializes the Vulkan instance.
 * 
 * The Vulkan instance bridges the gap between the Vulkan API and the application.
 * 
 * The function validates that the required layers and extensions are supported before instance
 * creation, then creates the instance with a Vulkan RAII object for automatic destruction.
 * 
 * @throws std::runtime_error if a required validation layer or extension is unsupported.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/01_Instance.html
 */
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

/**
 * @brief Gets the required Vulkan and GLFW extensions before Vulkan instance creation.
 * 
 * This function is used to get the required extensions during instance creation (see
 * Application::CreateInstance).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/01_Instance.html
 */
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

/**
 * @brief Initializes the debug messenger for the validation layers.
 * 
 * Because the Vulkan API is designed to have minimal driver overhead and be as performant as
 * possible, errors can be silent and will result in crashes or unexpected behavior. Therefore,
 * validation layers (they are optional) are used to check for errors.
 * 
 * The debug messenger returns error messages to the terminal that are caught by the validation
 * layers.
 * 
 * The severity of the message is specified with one of the following flags:
 * 
 * - vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
 *      Diagnostic message from Vulkan components (e.g. loader, layers, drivers)
 * 
 * - vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
 *      Informational message (e.g. creation of a resource)
 * 
 * - vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
 *      Message about behavior that may come from an application bug
 * 
 * - vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
 *      Message about behavior that is invalid
 * 
 * The message type is specified with one of the following values:
 * 
 * - vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral:
 *      Some event has happened that is unrelated to the specification or performance
 * 
 * - vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation:
 *      Something has happened that violates the specification or indicates a possible mistake
 * 
 * - vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance:
 *      Potential non-optimal use of Vulkan
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/02_Validation_layers.html
 */
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

/**
 * @brief Selects the best Vulkan physical device to use.
 * 
 * The Vulkan physical device object is only used for retrieving its properties and capabilities to
 * be used for operations. The Vulkan logical device (see Application::CreateLogicalDevice) uses
 * the physical device's features and will be used as the handle for Vulkan operations after the
 * physical device has been initiated.
 * 
 * The function selects the device candidate based on its "score," influenced by the device's
 * properties and if it is a discrete GPU.
 * 
 * @throws std::runtime_error if no suitable GPU was found.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/03_Physical_devices_and_queue_families.html
 */
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

/**
 * @brief Checks if a Vulkan physical device object has the required Vulkan extensions and features
 * needed for the application.
 * 
 * This function is used during Vulkan physical device selection (see
 * Application::SelectPhysicalDevice).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/03_Physical_devices_and_queue_families.html
 */
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

/**
 * @brief Creates the Vulkan logical device and specifies the queue families used.
 * 
 * The Vulkan logical device uses the physical device's features and is used as the handle for
 * Vulkan operations.
 * 
 * The Vulkan queue is an asynchronous execution queue that receives commands. Queues are decided
 * from queue families which represents a set of queues that support a specific set of operations.
 * 
 * @throws std::runtime_error if a Vulkan queue could not be found.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/04_Logical_device_and_queues.html
 */
void Application::CreateLogicalDevice() {
    // Get the first index of the queue with graphics capabilities
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

    // Bitwise operator; get the max possible uint32_t
    m_queueIndex = ~0;

    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
        if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
            m_physicalDevice.getSurfaceSupportKHR(qfpIndex, *m_surface)) {
            // Found a queue family that supports both Vulkan graphics and present surfaces
            m_queueIndex = qfpIndex;
            break;
        }
    }
    if (m_queueIndex == ~0) {
        throw std::runtime_error("Could not find a queue for Vulkan graphics and present surfaces!");
    }

    // Decide which queues have relative priority over other queues
    float queuePriorities[] = {
        0.5f
    };

    vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
        .queueFamilyIndex = m_queueIndex,
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
#ifdef POLYGON_FILL
            // Empty
#else
            // Anything other than vk::PolygonMode::eFill must enable the fillModeNonSolid feature
            .features = {
                .fillModeNonSolid = true
            }
#endif
        },
        /* Vulkan 1.1 features */
        {
            .shaderDrawParameters = true
        },
        /* Vulkan 1.3 feature */
        {
            .synchronization2 = true,
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
    m_queue = vk::raii::Queue(m_device, m_queueIndex, 0);
}

/**
 * @brief Creates the Vulkan-GLFW window surface.
 * 
 * The window surface allows Vulkan rendering to an OS window because the Vulkan API is platform-
 * agnostic and requires a standardized WSI (Window System Interface) with cross-platform support.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/00_Window_surface.html
 */
void Application::CreateSurface() {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(*m_instance, m_window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create the GLFW-Vulkan window surface!");
    }

    m_surface = vk::raii::SurfaceKHR(m_instance, surface);
}

/**
 * @brief Creates the Vulkan swap chain.
 * 
 * The Vulkan swap chain is a queue (most of the time) that swaps rendered images in its queue to
 * the window surface (see Application::CreateSurface). This way, only complete images are
 * displayed and rendering can occur before the image refreshes to prevent screen tearing (in the
 * commonly-used swap chain modes).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html
 */
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

/**
 * @brief Chooses a window surface (see Application::CreateSurface) format for the swap chain,
 * preferred by if color formatting is more accurate.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html
 */
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

/**
 * @brief Chooses a present mode for the swap chain.
 * 
 * The present mode for the window surface is used when displaying an image from the Vulkan queue (
 * see Application::CreateLogicalDevice).
 * 
 * The function chooses vk::PresentModeKHR::eMailbox (triple buffering) by default if available.
 * Otherwise, it uses vk::PresentModeKHR::eFifo (double buffering) which is guaranteed to be
 * available.
 * 
 * The following present modes can be used:
 * 
 * - vk::PresentModeKHR::eImmediate
 *      Swap chain images are displayed immediately which may result in screen tearing.
 * 
 * - vk::PresentModeKHR::eFifo
 *      The swap chain is a queue of images where the screen displays images refreshed from the
 *      queue. When the queue is full, it blocks the application. This mode is similar to
 *      vertical sync.
 * 
 * - vk::PresentModeKHR::eFifoRelaxed
 *      This mode is the same as the vk::PresentModeKHR::eFifo except when waiting for the
 *      queue to fill, the image is displayed immediately which may result in screen tearing.
 * 
 * - vk::PresentModeKHR::eMailbox
 *      This is another variation of the vk::PresentModeKHR::eFifo except when the queue is
 *      full images already in the queue are replaced with newer ones. This mode is commonly
 *      known as triple buffering. This mode can be slightly more demanding to use than the
 *      FIFO mode.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html
 */
vk::PresentModeKHR Application::ChooseSwapPresentMode(
    std::vector<vk::PresentModeKHR> const &availablePresentModes
) {
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

    // Make sure there is an available present mode
    assert(std::ranges::any_of(
        availablePresentModes,
        [](auto presentMode) {
            return presentMode == vk::PresentModeKHR::eFifo;
        }
    ));
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

/**
 * @brief Chooses a Vulkan window extent (screen size in pixels) to draw to the window surface (see
 * Application::CreateSurface).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html
 */
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

/**
 * @brief Chooses a specific minimum number of images to use in the swap chain (see
 * Application::CreateSwapChain).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html
 */
uint32_t Application::ChooseSwapMinImageCount(
    vk::SurfaceCapabilitiesKHR const &surfaceCapabilities
) {
    /**
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

/**
 * Create Vulkan ImageViews to fill up the swap chain.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/02_Image_views.html
 */
void Application::CreateImageViews() {
    // Make sure there is atleast an ImageView
    assert(m_swapChainImageViews.empty());

    vk::ImageViewCreateInfo imageViewCreateInfo{
        // 2D screen
        .viewType = vk::ImageViewType::e2D,
        .format = m_swapChainSurfaceFormat.format,
        .subresourceRange = {
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
        }
    };

    // The color channels can be swizzled around here
    imageViewCreateInfo.components = {
        /* Use the default mapping: vk::ComponentSwizzle::eIdentity */
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity
    };

    // The image's details and access can be described here
    imageViewCreateInfo.subresourceRange = {
        // Color target
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        // Use no mipmapping levels
        .levelCount = 1,
        // Use no multiple layers; only one layer
        .layerCount = 1
    };

    // Add the ImageViews to the swap chain
    for (auto &image : m_swapChainImages) {
        imageViewCreateInfo.image = image;
        // Add inplace to the end of the ImageViews to add to the swap chain
        m_swapChainImageViews.emplace_back(
            m_device,
            imageViewCreateInfo
        );
    }
}

/**
 * @brief Create a Vulkan shader module from code (in bytes).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/01_Shader_modules.html
 */
[[nodiscard]] vk::raii::ShaderModule Application::CreateShaderModule(const std::vector<char>& code) const {
    vk::ShaderModuleCreateInfo createInfo{
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t *>(code.data())
    };
    vk::raii::ShaderModule shaderModule{m_device, createInfo};

    return shaderModule;
}

/**
 * @brief Create the graphics pipeline for rendering.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/00_Introduction.html
 */
void Application::CreateGraphicsPipeline() {
    auto shaderCodeMainVert = ReadFile(SHADER_MAIN_VERT_PATH);
    auto shaderCodeMainFrag = ReadFile(SHADER_MAIN_FRAG_PATH);

    vk::raii::ShaderModule shaderModuleMainVert = CreateShaderModule(shaderCodeMainVert);
    vk::raii::ShaderModule shaderModuleMainFrag = CreateShaderModule(shaderCodeMainFrag);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shaderModuleMainVert,
        .pName = SHADER_ENTRY_POINT
    };
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shaderModuleMainFrag,
        .pName = SHADER_ENTRY_POINT
    };
    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo
    };

    std::vector<vk::DynamicState> dynamicStates = {
        /* Allow these states to be updated during runtime */
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
#ifdef TOPOLOGY_POINT_LIST
        .topology = vk::PrimitiveTopology::ePointList,
#endif
#ifdef TOPOLOGY_LINE_LIST
        .topology = vk::PrimitiveTopology::eLineList,
#endif
#ifdef TOPOLOGY_LINE_STRIP
        .topology = vk::PrimitiveTopology::eLineStrip,
#endif
#ifdef TOPOLOGY_TRIANGLE_LIST
        .topology = vk::PrimitiveTopology::eTriangleList,
#endif
#ifdef TOPOLOGY_TRIANGLE_STRIP
        .topology = vk::PrimitiveTopology::eTriangleStrip
#endif
    };
    vk::PipelineViewportStateCreateInfo viewportState{
        .viewportCount = 1,
        .scissorCount = 1
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
#ifdef POLYGON_FILL
        .polygonMode = vk::PolygonMode::eFill,
#endif
#ifdef POLYGON_LINE
        .polygonMode = vk::PolygonMode::eLine,
#endif
#ifdef POLYGON_POINT
        .polygonMode = vk::PolygonMode::ePoint,
#endif
        .cullMode = vk::CullModeFlagBits::eNone,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{
        /**
         * Disable multisampling.
         * 
         * NOTE: enabling this feature in the future will require a GPU feature.
         */
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        /**
         * Disable color blending.
         */
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0
    };

    m_pipelineLayout = vk::raii::PipelineLayout(m_device, pipelineLayoutInfo);

    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &m_swapChainSurfaceFormat.format
    };
    vk::GraphicsPipelineCreateInfo graphicsRenderingCreateInfo{
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = m_pipelineLayout,
        // Set to nullptr because the render passes will be dynamic
        .renderPass = nullptr
    };

    vk::StructureChain<
        vk::GraphicsPipelineCreateInfo,
        vk::PipelineRenderingCreateInfo
    > pipelineCreateInfoChain = {
        graphicsRenderingCreateInfo,
        pipelineRenderingCreateInfo
    };

    m_graphicsPipeline = vk::raii::Pipeline(
        m_device,
        nullptr,
        pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>()
    );
}

/**
 * @brief Create a Vulkan command pool.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void Application::CreateCommandPool() {
    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_queueIndex
    };

    m_commandPool = vk::raii::CommandPool(m_device, poolInfo);
}

/**
 * @brief Allocate a Vulkan command buffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void Application::CreateCommandBuffer() {
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = m_commandPool,
        /**
         * The level determines if the command buffer is a primary or secondary command buffer.
         * 
         * - vk::CommandBufferLevel::ePrimary:
         *      Can be submitted to a command queue.
         * 
         * - vk::CommandBufferLevel::eSecondary:
         *      Cannot be submitted to a command queue but can be called from primary command
         *      buffers.
         */
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };

    m_commandBuffer = std::move(vk::raii::CommandBuffers(m_device, allocInfo).front());
}

/**
 * @brief Record a Vulkan command buffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void Application::RecordCommandBuffer(uint32_t imageIndex) {
    m_commandBuffer.begin({});
    
    // Swapchain image: undefined -> vk::ImageLayout::eColorAttachmentOptimal
    TransitionImageLayout(
        imageIndex,
        vk::ImageLayout::eUndefined, // Old layout
        vk::ImageLayout::eColorAttachmentOptimal, // New layout
        {}, // srcAccessMask
        vk::AccessFlagBits2::eColorAttachmentWrite, // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput // dstStage
    );

    vk::ClearValue clearColor = vk::ClearColorValue(
        /* Black color */
        0.0f, 0.0f, 0.0f, 1.0f
    );

    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = m_swapChainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        // Image preprocessing
        .loadOp = vk::AttachmentLoadOp::eClear,
        // Image postprocessing
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::RenderingInfo renderingInfo = {
        .renderArea = {
            .offset = {0, 0},
            .extent = m_swapChainExtent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    m_commandBuffer.beginRendering(renderingInfo);
    m_commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        *m_graphicsPipeline
    );
    m_commandBuffer.setViewport(
        0,
        vk::Viewport(
            0.0f,
            0.0f,
            static_cast<float>(m_swapChainExtent.width),
            static_cast<float>(m_swapChainExtent.height),
            0.0f,
            1.0f
        )
    );
    m_commandBuffer.setScissor(
        0,
        vk::Rect2D(
            vk::Offset2D(0, 0),
            m_swapChainExtent
        )
    );

    m_commandBuffer.draw(3, 1, 0, 0);

    m_commandBuffer.endRendering();

    // Swapchain image: vk::ImageLayout::eColorAttachmentOptimal -> vk::ImageLayout::ePresentSrcKHR
    TransitionImageLayout (
        imageIndex,
        vk::ImageLayout::eColorAttachmentOptimal, // Old layout
        vk::ImageLayout::ePresentSrcKHR, // New layout
        vk::AccessFlagBits2::eColorAttachmentWrite, // srcAccessMask
        {}, // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe // dstStage
    );

    m_commandBuffer.end();
}

/**
 * @brief Transition a Vulkan image layout to and from being suitable for rendering.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void Application::TransitionImageLayout(
    uint32_t imageIndex,
    vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout,
    vk::AccessFlags2 srcAccessMask,
    vk::AccessFlags2 dstAccessMask,
    vk::PipelineStageFlags2 srcStageMask,
    vk::PipelineStageFlags2 dstStageMask
) {
	vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_swapChainImages[imageIndex],
        .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1}};
    
    vk::DependencyInfo dependency_info = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier};
    
    m_commandBuffer.pipelineBarrier2(dependency_info);
}

/**
 * @brief Draw a frame.
 * 
 * Called from the main loop.
 * 
 * A Vulkan sephamore is a synchronization object used to order GPU queue operations (GPU -> GPU).
 * 
 * A Vulkan fence is a synchronization object used to order CPU queue operations (CPU -> CPU).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html
 */
void Application::DrawFrame() {
    /*  Wait until the previous frame is finished */
    auto fenceResult = m_device.waitForFences(
        *m_drawFence,
        vk::True,
        UINT64_MAX // Timeout
    );
    if (fenceResult != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence!");
    }
    m_device.resetFences(*m_drawFence);

    auto [result, imageIndex] = m_swapChain.acquireNextImage(
        UINT64_MAX, // Timeout
        *m_presentCompleteSemaphore,
        nullptr
    );

    RecordCommandBuffer(imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    );
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_presentCompleteSemaphore,
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*m_commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*m_renderFinishedSemaphore
    };

    m_queue.submit(submitInfo, *m_drawFence);

    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &*m_swapChain,
        .pImageIndices = &imageIndex
    };

    result = m_queue.presentKHR(presentInfoKHR);
}

/**
 * @brief Draw a frame.
 * 
 * Called from the main loop.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html
 */
void Application::CreateSyncObjects() {
    m_presentCompleteSemaphore = vk::raii::Semaphore(
        m_device,
        vk::SemaphoreCreateInfo()
    );
    m_renderFinishedSemaphore = vk::raii::Semaphore(
        m_device,
        vk::SemaphoreCreateInfo()
    );
    m_drawFence = vk::raii::Fence(
        m_device,
        {.flags = vk::FenceCreateFlagBits::eSignaled}
    );
}
