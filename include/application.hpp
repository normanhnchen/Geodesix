/**
 * ============================================================
 * Adapted from the official Vulkan Tutorial
 * https://docs.vulkan.org/tutorial/latest/00_Introduction.html
 * ============================================================
 */


#pragma once


#include <vector>
#include <iostream>
#include <cstdint>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

/* Debug definitions */
#define DEBUG_VALIDATION_LAYERS
// #define DEBUG_PRINT_EXTENSIONS


constexpr uint32_t WIDTH  = 800;
constexpr uint32_t HEIGHT = 600;

const std::vector<char const*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

class Application {
public:
    void Run();

private:
    GLFWwindow* m_window = nullptr;
    vk::raii::Context m_context;
	vk::raii::Instance m_instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger = nullptr;
    vk::raii::PhysicalDevice m_physicalDevice = nullptr;
    vk::raii::Device m_device = nullptr;
    vk::raii::Queue m_graphicsQueue = nullptr;

    std::vector<const char*> requiredDeviceExtension = {
        vk::KHRSwapchainExtensionName
    };

    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void Cleanup();

    void CreateInstance();
    void SetupDebugMessenger();
    std::vector<const char*> GetRequiredInstanceExtensions();

    // Debug messenger (callback) for the validation layers
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
        /**
         * The severity of the message is specified with one of the following flags
         * - vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
         *      Diagnostic message from Vulkan components (e.g. loader, layers, drivers)
         * - vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
         *      Informational message (e.g. creation of a resource)
         * - vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
         *      Message about behavior that may come from an application bug
         * - vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
         *      Message about behavior that is invalid
         */
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        /**
         * The message type is specified with one of the following values
         * - vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral:
         *      Some event has happened that is unrelated to the specification or performance
         * - vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation:
         *      Something has happened that violates the specification or indicates a possible mistake
         * - vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance:
         *      Potential non-optimal use of Vulkan
         */
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
        void * pUserData
    ) {
        if (
            severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning ||
            severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
        ) {
            // Message is important enough to show because of the severity
            std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
        }

        return vk::False;
    }

    void SelectPhysicalDevice();
    bool IsDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);

    void CreateLogicalDevice();
};
