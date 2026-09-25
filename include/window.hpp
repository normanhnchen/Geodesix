#pragma once


#include "header_inclusions/vulkan.hpp"
#include "header_inclusions/glfw.hpp"


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
