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


class SwapChain {
public:
    SwapChain(Window& window, VulkanContext& vulkanContext);

    void Init();

private:
    Window& m_window;
    VulkanContext& m_vulkanContext;
    vk::raii::SwapchainKHR m_swapChain = nullptr;
    std::vector<vk::Image> m_swapChainImages;
    vk::SurfaceFormatKHR m_swapChainSurfaceFormat;
    vk::Extent2D m_swapChainExtent;
    std::vector<vk::raii::ImageView> m_swapChainImageViews;

    void SwapChain::CreateSwapChain(
        const vk::raii::PhysicalDevice& physicalDevice,
        const vk::raii::Device& device,
        const vk::raii::SurfaceKHR& surface
    );
    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
    vk::PresentModeKHR ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
    vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
    uint32_t ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

    void CreateImageViews(const vk::raii::Device& device);
};
