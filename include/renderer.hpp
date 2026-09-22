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
