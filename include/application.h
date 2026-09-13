# pragma once

#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>


class Application {
public:
    /* ---- Public Functions ---- */
    
    void Initialize();
    bool IsRunning();
    void MainLoop();
    void Terminate();

private:
    /* ---- Private Members ----*/

    GLFWwindow* m_window = nullptr;
    WGPUInstance m_instance = nullptr;
    WGPUSurface m_surface = nullptr;
    WGPUAdapter m_adapter = nullptr;
    WGPUDevice m_device = nullptr;
    WGPUQueue m_queue = nullptr;
    WGPUCommandEncoder m_encoder = nullptr;
    WGPUCommandBuffer m_command = nullptr;

    /* Initialization functions */

    GLFWwindow* InitWindow();
    WGPUInstance InitInstance();
    WGPUSurface InitSurface();
    WGPUAdapter InitAdapter();
    WGPUDevice InitDevice();
    WGPUQueue InitQueue();

    /* Main loop functions */

    WGPUCommandEncoder CreateCommandEncoder();
    WGPUCommandBuffer FinishCommandEncoder();
    void SubmitCommandBuffer();
    void WaitForQueueDone();

    /* Status & error check functions */

    void CheckGlfwInit(int success);
    void CheckGlfwWindow(GLFWwindow* window);
    void CheckWgpuInstance(WGPUInstance instance);
    void CheckWgpuSurface(WGPUSurface surface);
    void CheckWgpuAdapter(WGPUAdapter adapter);
    void CheckWgpuDevice(WGPUDevice device);
    void CheckWgpuQueue(WGPUQueue queue);
    void CheckWgpuCommandEncoder(WGPUCommandEncoder encoder);
    void CheckWgpuCommandBuffer(WGPUCommandBuffer command);
};