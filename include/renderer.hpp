#pragma once


#include <iostream>
#include <vector>
#include <stdexcept>

#include "header_inclusions/vulkan.hpp"

#include "window.hpp"
#include "vulkan_context.hpp"
#include "swap_chain.hpp"
#include "pipeline.hpp"
#include "command_context.hpp"
#include "sync_context.hpp"
#include "buffer_context.hpp"


class Renderer {
public:
    Renderer(
        Window& window,
        VulkanContext& vulkanContext,
        SwapChain& swapChain,
        Pipeline& pipeline,
        CommandContext& commandContext,
        SyncContext& syncContext,
        BufferContext& bufferContext
    );

    void Init();

    void DrawFrame();

private:
    Window& m_window;
    VulkanContext& m_vulkanContext;
    SwapChain& m_swapChain;
    Pipeline& m_pipeline;
    CommandContext& m_commandContext;
    SyncContext& m_syncContext;
    BufferContext& m_bufferContext;

    uint32_t m_frameIndex = 0;
};
