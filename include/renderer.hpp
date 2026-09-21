#pragma once


#include <iostream>
#include <vector>
#include <stdexcept>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "window.hpp"
#include "vulkan_context.hpp"
#include "swap_chain.hpp"
#include "pipeline.hpp"


constexpr int MAX_FRAMES_IN_FLIGHT = 2;

class Renderer {
public:
    Renderer(
        Window& window,
        VulkanContext& vulkanContext,
        SwapChain& swapChain,
        Pipeline& pipeline
    );

    void Init();

    void DrawFrame();

private:
    Window& m_window;
    VulkanContext& m_vulkanContext;
    SwapChain& m_swapChain;
    Pipeline& m_pipeline;

    vk::raii::CommandPool m_commandPool = nullptr;
    std::vector<vk::raii::CommandBuffer> m_commandBuffers;
    std::vector<vk::raii::Semaphore> m_presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_inFlightFences;

    uint32_t m_frameIndex = 0;

    void CreateGraphicsPipeline();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void RecordCommandBuffer(uint32_t imageIndex);
    void TransitionImageLayout(
        uint32_t imageIndex,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask
    );

    void CreateSyncObjects();
};
