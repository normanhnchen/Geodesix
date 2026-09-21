#pragma once


#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {
public:
    Window(int width, int height, const char* title);

    void Init();
    void Cleanup();

    bool ShouldClose();
    void PollEvents();

    void GetFramebufferSize(int* width, int* height);

    bool Resized();
    void ResetResizedFlag();

    void MinimizedLoop();

    VkSurfaceKHR CreateVulkanSurface(VkInstance instance);

private:

    int m_width;
    int m_height;
    const char* m_title = nullptr;

    GLFWwindow* m_window = nullptr;
    bool m_framebufferResized = false;

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
};
