#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application.hpp"


void Application::Run() {
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}

void Application::InitWindow() {
    glfwInit();

    // Since GLFW creates an OpenGL context by default, we tell it to not create one
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    /**
     * !!!!!!!!!!!!!!!!!!!!!
     * NOTE: disable for now
     * !!!!!!!!!!!!!!!!!!!!!
     */
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(800, 600, "Geodesix", nullptr, nullptr);
}

void Application::InitVulkan() {

}

void Application::MainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void Application::Cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}
