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


/* ==== Debug Macros ==== */

#define DEBUG_VALIDATION_LAYERS
// #define DEBUG_PRINT_EXTENSIONS

/**
 * ---- Present Modes ----
 * These macros are used in Application::ChooseSwapPresentMode
 */
// #define DEBUG_PRESENT_IMMEDIATE
// #define DEBUG_PRESENT_FIFO
// #define DEBUG_PRESENT_FIFO_RELAXED
// #define DEBUG_PRESENT_MAILBOX

/**
 * The macro below defines the minimum image count strategy used in
 * Application::ChooseSwapMinImageCount. If the macro is defined,  we use the tutorial's
 * explanatory minimum count minImageCount + 1. Otherwise, we use the tutorial's shipped version
 * std::max(3u, surfaceCapabilities.minImageCount).
 */
// #define DEBUG_MIN_IMAGE_COUNT_LEGACY


constexpr uint32_t WIDTH  = 800;
constexpr uint32_t HEIGHT = 600;

const std::vector<char const*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

/**
 * The main application, including a Vulkan & GLFW backend.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
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
    vk::raii::SurfaceKHR m_surface = nullptr;
    vk::raii::SwapchainKHR m_swapChain = nullptr;
    std::vector<vk::Image> m_swapChainImages;
    vk::SurfaceFormatKHR m_swapChainSurfaceFormat;
    vk::Extent2D m_swapChainExtent;

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

    /**
     * @brief Debug messenger callback for the validation layers.
     * 
     * See Application::SetupDebugMessenger for the debug messenger creation.
     * 
     * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/02_Validation_layers.html
     */
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
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

    void CreateSurface();

    void CreateSwapChain();
    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
    vk::PresentModeKHR ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
    vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
    uint32_t ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);
};
