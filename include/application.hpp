/**
 * ============================================================
 * Adapted from the official Vulkan Tutorial
 * https://docs.vulkan.org/tutorial/latest/00_Introduction.html
 * ============================================================
 */


#pragma once


class Application {
public:
    void Run();

private:
    GLFWwindow* window = nullptr;

    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void Cleanup();
};
