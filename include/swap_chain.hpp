#pragma once


#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "window.hpp"
#include "vulkan_context.hpp"


/* ==== Debug Macros ==== */

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

class SwapChain {
public:
    SwapChain(Window& window, VulkanContext& vulkanContext);

    void Init();
    void Cleanup();

    void Recreate();

    std::optional<uint32_t> AcquireNextImageIndex(
        const vk::raii::Semaphore& presentCompleteSemaphore
    );

    const vk::raii::SwapchainKHR& GetNative() const;
    std::vector<vk::Image> GetImages();
    vk::Extent2D GetExtent();
    const std::vector<vk::raii::ImageView>& GetImageViews() const;
    vk::SurfaceFormatKHR GetSurfaceFormat();

private:
    Window& m_window;
    VulkanContext& m_vulkanContext;
    vk::raii::SwapchainKHR m_swapChain = nullptr;
    std::vector<vk::Image> m_swapChainImages;
    vk::SurfaceFormatKHR m_swapChainSurfaceFormat;
    vk::Extent2D m_swapChainExtent;
    std::vector<vk::raii::ImageView> m_swapChainImageViews;

    void CreateSwapChain();
    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
    vk::PresentModeKHR ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
    vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
    uint32_t ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

    void CreateImageViews();
};
