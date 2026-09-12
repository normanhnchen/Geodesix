#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include <glfw3webgpu.h>
#include <iostream>
#include <cassert>

#include "request.h"
#include "inspect.h"
#include "util.h"


// Adapted from LearnWebGPU-Code (MIT License)
// See THIRD_PARTY_NOTICES.md#learnwebgpu-code
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
        wgpuSurfaceRelease(surface);
        wgpuInstanceRelease(instance);
        glfwTerminate();
        return -1;
    }

    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = nullptr;
    deviceDesc.label.data = "Device";
    deviceDesc.requiredFeatureCount = 0; // We do not require any specific feature
    deviceDesc.requiredLimits = nullptr; // We do not require any specific limit
    deviceDesc.defaultQueue.nextInChain = nullptr;

    WGPUDevice device = requestDeviceSync(instance, adapter, &deviceDesc);

    if (!device) {
        std::cerr << "Failed to find a device!" << std::endl;
        wgpuSurfaceRelease(surface);
        wgpuInstanceRelease(instance);
        glfwTerminate();
    }

    WGPUQueue queue = wgpuDeviceGetQueue(device);

    if (!queue) {
        std::cerr << "Failed to get the command queue!" << std::endl;
        wgpuAdapterRelease(adapter);
        wgpuSurfaceRelease(surface);
        wgpuInstanceRelease(instance);
        glfwTerminate();
        return -1;
    }

    // inspectAdapter(adapter);
    // inspectDevice(device);

    /* Window loop */
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Command Encoder
        // ---------------
        WGPUCommandEncoderDescriptor encoderDesc = {};
        encoderDesc.label.data = "Command Encoder";
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

        if (!encoder) {
            std::cerr << "Failed to get the command encoder!" << std::endl;
            break;
        }

        wgpuCommandEncoderInsertDebugMarker(encoder, toWgpuStringView("Do something"));
        
        // Command Buffer
        // --------------
        WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
        cmdBufferDescriptor.label = toWgpuStringView("Command buffer");
        WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
        // Release the encoder after it is finished
        wgpuCommandEncoderRelease(encoder);

        // Submit the command queue
        wgpuQueueSubmit(queue, 1, &command);
        // Release the command buffer after it is done
        wgpuCommandBufferRelease(command);

        auto onQueuedWorkDone = [](
            WGPUQueueWorkDoneStatus status,
            void* userdata1,
            void* // userdata2: not needed for this callback
        ) {
            // Display a warning when status is not success
            if (status != WGPUQueueWorkDoneStatus_Success) {
                std::cout << "Warning: wgpuQueueOnSubmittedWorkDone failed!" << std::endl;
            } else {
                std::cout << "Frame finished on GPU" << std::endl;
            }

            // Interpret userdata1 as a pointer to a boolean (and turn it into a
            // mutable reference), then turn it to 'true'
            bool& workDone = *reinterpret_cast<bool*>(userdata1);
            workDone = true;
        };

        bool workDone = false;

        WGPUQueueWorkDoneCallbackInfo callbackInfo = {};
        callbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
        callbackInfo.callback = onQueuedWorkDone;
        callbackInfo.userdata1 = &workDone;

        // Add the async operation to the queue
        wgpuQueueOnSubmittedWorkDone(queue, callbackInfo);

        // Check for pending async operations
        wgpuInstanceProcessEvents(instance);
        while (!workDone) {
            wgpuInstanceProcessEvents(instance);
        }
    }

    wgpuQueueRelease(queue);
    wgpuDeviceRelease(device);
    wgpuAdapterRelease(adapter);
    wgpuSurfaceRelease(surface);
    wgpuInstanceRelease(instance);
    glfwTerminate();

    return 0;
}
