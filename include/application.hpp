#pragma once


#include <vector>
#include <iostream>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>

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
#include "vulkan_context.hpp"
#include "swap_chain.hpp"
#include "renderer.hpp"


constexpr uint32_t WIDTH  = 800;
constexpr uint32_t HEIGHT = 600;

/**
 * The main application, including a Vulkan & GLFW backend.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
class Application {
public:
    void Run();

private:
    Window m_window {WIDTH, HEIGHT, "Geodesix"};
    VulkanContext m_vulkanContext {m_window};
    SwapChain m_swapChain {m_window, m_vulkanContext};
    Pipeline m_pipeline {m_vulkanContext, m_swapChain};
    Renderer m_renderer {m_window, m_vulkanContext, m_swapChain, m_pipeline};

    void InitVulkan();
    void MainLoop();
    void Cleanup();
};
