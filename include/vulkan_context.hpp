#pragma once


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
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "window.hpp"


/* ==== Debug Macros ==== */

#define DEBUG_VALIDATION_LAYERS
// #define DEBUG_PRINT_EXTENSIONS

const std::vector<char const*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};


class VulkanContext {
public:
    VulkanContext(Window& window);

    void Init();

    void WaitForDevice();

private:
    Window m_window;
    vk::raii::Context m_context;
    vk::raii::Instance m_instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger = nullptr;
    vk::raii::PhysicalDevice m_physicalDevice = nullptr;
    vk::raii::Device m_device = nullptr;
    vk::raii::Queue m_queue = nullptr;
    vk::raii::SurfaceKHR m_surface = nullptr;

    uint32_t m_queueIndex = 0;

    std::vector<const char*> requiredDeviceExtension = {
        vk::KHRSwapchainExtensionName
    };

    void CreateInstance();
    void SetupDebugMessenger();
    std::vector<const char*> GetRequiredInstanceExtensions();

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );

    void SelectPhysicalDevice();
    bool IsDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);

    void CreateLogicalDevice();

    void CreateSurface();
};
