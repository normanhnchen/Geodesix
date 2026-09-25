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

#include "header_inclusions/vulkan.hpp"
#include "header_inclusions/glfw.hpp"

#include "window.hpp"


/* ==== Debug Macros ==== */

#define DEBUG_VALIDATION_LAYERS
// #define DEBUG_PRINT_EXTENSIONS

/**
 * ---- Polygon Mode ----
 * 
 * These macros are used in Renderer::CreateGraphicsPipeline for the rasterizer struct.
 * 
 * NOTE: only one of the macros should be defined or else it might lead to unexpected behavior!
 */

/**
 * Fill the area of polygons.
 * 
 * vk::PolygonMode::eFill
 */
#define POLYGON_FILL
/**
 * Draw polygon edges as lines.
 * 
 * vk::PolygonMode::eLine
 */
// #define POLYGON_LINE
/**
 * Draw polygon vertices as points.
 * 
 * vk::PolygonMode::ePoint
 */
// #define POLYGON_POINT

const std::vector<char const*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

class VulkanContext {
public:
    VulkanContext(Window& window);

    void Init();

    void WaitForDevice();

    const vk::raii::PhysicalDevice& GetPhysicalDevice() const;
    const vk::raii::Device& GetDevice() const;
    const vk::raii::SurfaceKHR& GetSurface() const;
    const vk::raii::Queue& GetQueue() const;
    uint32_t GetQueueIndex();

private:
    Window& m_window;
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
