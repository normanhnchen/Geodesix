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

#include "application.hpp"


/**
 * @brief Runs the application.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
void Application::Run() {
    m_window.Init();
    InitVulkan();
    MainLoop();
    Cleanup();
}

/**
 * @brief Initializes the Vulkan library and calls all of the required helper initialization
 * functions in the *required dependency order*:
 * 
 * (Instance -> (Validation Layers & Debug messenger) -> Surface -> Physical Device -> Logical
 *  Device -> Swap Chain -> Image Views)
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
void Application::InitVulkan() {
    m_vulkanContext.Init();
    m_swapChain.Init();
    m_renderer.Init();
}

/**
 * @brief Main rendering loop.
 * 
 * TODO: currently only polls GLFW events; add actual rendering implementations and Vulkan API
 * calls.
 */
void Application::MainLoop() {
    while (!m_window.ShouldClose()) {
        m_window.PollEvents();
        m_renderer.DrawFrame();
    }

    // Wait for the logical device to finish its operations before terminating
    m_vulkanContext.WaitForDevice();
}

/**
 * @brief Destroys the window and terminates GLFW before terminating the program. Vulkan resources
 * are cleaned up via RAII.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
void Application::Cleanup() {
    m_swapChain.Cleanup();
    m_window.Cleanup();
}
