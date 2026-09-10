#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include <glfw3webgpu.h>
#include <iostream>
#include <cassert>

#include "request.h"
#include "inspect.h"


// Adapted from LearnWebGPU-Code by Élie Michel (https://github.com/eliemichel/LearnWebGPU-Code)
// MIT License
int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }
    
    // Tell GLFW to not create an OpenGL context (by default)
    // WGPU manages its own connection to the GPU
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Geodesix", nullptr, nullptr);

    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    WGPUInstance instance = wgpuCreateInstance(nullptr);
    
    if (!instance) {
        std::cerr << "Failed to create WGPU instance!" << std::endl;
        glfwTerminate();
        return -1;
    }

    WGPUSurface surface = glfwCreateWindowWGPUSurface(instance, window);

    if (!surface) {
        std::cerr << "Failed to create WGPU surface!" << std::endl;
        wgpuInstanceRelease(instance);
        glfwTerminate();
        return -1;
    }

    WGPURequestAdapterOptions adapterOpts = {};
    adapterOpts.nextInChain = nullptr;
    adapterOpts.compatibleSurface = surface;
    WGPUAdapter adapter = requestAdapterSync(instance, &adapterOpts);

    if (!adapter) {
        std::cerr << "Failed to find a WebGPU adapter!" << std::endl;
        return -1;
    }

    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = nullptr;
    deviceDesc.requiredFeatureCount = 0; // We do not require any specific feature
    deviceDesc.requiredLimits = nullptr; // We do not require any specific limit
    deviceDesc.defaultQueue.nextInChain = nullptr;

    WGPUDevice device = requestDeviceSync(instance, adapter, &deviceDesc);

    WGPUQueue queue = wgpuDeviceGetQueue(device);

    if (!queue) {
        std::cerr << "Failed to get the command queue!" << std::endl;
        return -1;
    }

    // inspectAdapter(adapter);
    // inspectDevice(device);

    /* Window loop */
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    wgpuQueueRelease(queue);
    wgpuDeviceRelease(device);
    wgpuAdapterRelease(adapter);
    wgpuSurfaceRelease(surface);
    wgpuInstanceRelease(instance);
    glfwTerminate();

    return 0;
}
