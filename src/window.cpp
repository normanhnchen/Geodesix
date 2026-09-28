#include "window.hpp"


/**
 * @brief Initializes the GLFW window.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
Window::Window(Camera& camera)
    : m_camera(camera) {
}

void Window::SetResolution(int width, int height) {
    m_width = width;
    m_height = height;
}

void Window::SetTitle(const char* title) {
    m_title = title;
}

/**
 * @brief Initializes the GLFW window.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
void Window::Init() {
    glfwInit();

    // Since GLFW creates an OpenGL context by default, we tell it to not create one
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, m_title, nullptr, nullptr);

    // Attach an arbitrary pointer to the window so the GLFW callback functions can access the
    // member variables
    glfwSetWindowUserPointer(m_window, this);

    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
    glfwSetCursorPosCallback(m_window, CursorPosCallback);
}

void Window::Cleanup() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool Window::ShouldClose() {
    return glfwWindowShouldClose(m_window);
}

void Window::PollEvents() {
    glfwPollEvents();
}

/**
 * @brief Get the actual window screen size in pixels.
 */
void Window::GetFramebufferSize(int* width, int* height) {
    glfwGetFramebufferSize(m_window, width, height);
}

/**
 * @brief Returns the m_framebufferResized flag.
 */
bool Window::Resized() {
    return m_framebufferResized;
}

/**
 * @brief Sets the m_framebufferResized flag to false.
 */
void Window::ResetResizedFlag() {
    m_framebufferResized = false;
}

void Window::MinimizedLoop() {
    /* Pause until the window is unminimized */
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    while (
        (width == 0 || height == 0) &&
        !glfwWindowShouldClose(m_window)
    ) {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }
}

/**
 * @brief Creates the Vulkan window surface.
 * 
 * The window surface allows Vulkan rendering to an OS window because the Vulkan API is platform-
 * agnostic and requires a standardized WSI (Window System Interface) with cross-platform support.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/00_Window_surface.html
 */
VkSurfaceKHR Window::CreateVulkanSurface(VkInstance instance) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, m_window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create the GLFW-Vulkan window surface!");
    }

    return surface;
}

/**
 * @brief Callback function for resizing the GLFW framebuffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/04_Swap_chain_recreation.html
 */
void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    self->m_framebufferResized = true;
}

void Window::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    self->ProcessMouseMovement(xpos, ypos);
}

void Window::ProcessMouseMovement(double xpos, double ypos) {
    if (m_firstMouse) {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
    }

    double dx = xpos - m_lastX;
    // Reversed; Vulkan window y dimension grows downwards
    double dy = m_lastY - ypos;
    m_lastX = xpos;
    m_lastY = ypos;

    m_camera.m_yaw += static_cast<float>(dx) * m_camera.m_sensitivity;
    m_camera.m_pitch += static_cast<float>(dy) * m_camera.m_sensitivity;
    m_camera.m_pitch = glm::clamp(m_camera.m_pitch, -89.99f, 89.99f);

    m_camera.UpdateVectors();
}
