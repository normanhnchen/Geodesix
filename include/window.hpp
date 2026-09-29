#pragma once


#include "header_inclusions/vulkan.hpp"
#include "header_inclusions/glfw.hpp"

#include "camera.hpp"


// Forward declaration
// Used because BufferContext and Window circularly depend on each other
class BufferContext;

class Window {
public:
    Window(Camera& camera);

    void RetrieveBufferContext(BufferContext& bufferContext);

    void SetResolution(int width, int height);
    void SetTitle(const char* title);

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
    Camera& m_camera;
    BufferContext* m_bufferContext = nullptr;

    int m_width;
    int m_height;
    const char* m_title = nullptr;

    GLFWwindow* m_window = nullptr;
    bool m_framebufferResized = false;

    double m_lastX = 0.0, m_lastY = 0.0;
    // Prevent camera snapping on launch or window resize
    bool m_firstMouse = true;

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void ProcessMouseMovement(double xpos, double ypos);
    void ProcessMouseScroll(double xoffset, double yoffset);
};
