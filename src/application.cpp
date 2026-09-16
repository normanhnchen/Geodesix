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

    window = glfwCreateWindow(WIDTH, HEIGHT, "Geodesix", nullptr, nullptr);
}

void Application::InitVulkan() {
    CreateInstance();
    SetupDebugMessenger();
}

void Application::MainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void Application::Cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::CreateInstance() {
    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName = "Hello Triangle",
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
    auto layerProperties = context.enumerateInstanceLayerProperties();
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
    auto extensions = context.enumerateInstanceExtensionProperties();
    
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

    instance = vk::raii::Instance(context, createInfo);
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
    debugMessenger = instance.createDebugUtilsMessengerEXT(
        debugUtilsMessengerCreateInfoEXT
    );
}
