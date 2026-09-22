#pragma once


#include <iostream>
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "vulkan_context.hpp"
#include "swap_chain.hpp"
#include "pipeline.hpp"
#include "sync_context.hpp"
#include "buffer_context.hpp"


class CommandContext {
public:
    CommandContext(
        VulkanContext& vulkanContext,
        SwapChain& swapChain,
        Pipeline& pipeline,
        SyncContext& syncContext,
        BufferContext& BufferContext
    );

    void Init();

    void RecordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex);

    const std::vector<vk::raii::CommandBuffer>& GetCommandBuffers() const;

private:
    VulkanContext& m_vulkanContext;
    SwapChain& m_swapChain;
    Pipeline& m_pipeline;
    SyncContext& m_syncContext;
    BufferContext& m_bufferContext;

    vk::raii::CommandPool m_commandPool = nullptr;
    std::vector<vk::raii::CommandBuffer> m_commandBuffers;

    void CreateCommandPool();
    void CreateCommandBuffer();
};
