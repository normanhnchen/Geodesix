#pragma once


#include <vector>
#include <iostream>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "header_inclusions/vulkan.hpp"
#include "header_inclusions/glfw.hpp"

#include "window.hpp"
#include "vulkan_context.hpp"
#include "swap_chain.hpp"
#include "pipeline.hpp"
#include "command_context.hpp"
#include "sync_context.hpp"
#include "buffer_context.hpp"
#include "renderer.hpp"
#include "camera.hpp"


/**
 * The main application, including a Vulkan & GLFW backend.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
class Application {
public:
    void Run();

private:
    Camera m_camera {};
    Window m_window {m_camera};
    VulkanContext m_vulkanContext {m_window};
    SwapChain m_swapChain {m_window, m_vulkanContext};
    SyncContext m_syncContext {m_vulkanContext, m_swapChain};
    BufferContext m_bufferContext {m_vulkanContext, m_syncContext, m_swapChain, m_window, m_camera};
    Pipeline m_pipeline {m_vulkanContext, m_swapChain, m_bufferContext};
    CommandContext m_commandContext {m_vulkanContext, m_swapChain, m_pipeline, m_syncContext, m_bufferContext};
    Renderer m_renderer {
        m_window,
        m_vulkanContext,
        m_swapChain,
        m_pipeline,
        m_commandContext,
        m_syncContext,
        m_bufferContext
    };

    void InitVulkan();
    void MainLoop();
    void Cleanup();
};
