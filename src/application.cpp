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
    SelectPhysicalDevice();
    CreateLogicalDevice();
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
    /* Get a queue with graphics capabilities */
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();
    auto graphicsQueueFamilyProperty = std::ranges::find_if(
        queueFamilyProperties,
        [](auto const &qfp) {
            return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
        }
    );
    auto graphicsIndex = static_cast<uint32_t>(std::distance(
        queueFamilyProperties.begin(),
        graphicsQueueFamilyProperty
    ));

    // Decide which queues have relative priority over other queues
    float queuePriorities[] = {
        0.5f
    };

    vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
        .queueFamilyIndex = graphicsIndex,
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
    m_graphicsQueue = vk::raii::Queue(m_device, graphicsIndex, 0);
}
