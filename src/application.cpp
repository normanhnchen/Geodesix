#include "application.h"
#include "request.h"
#include "util.h"

#include <webgpu/webgpu.h>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#include <iostream>
#include <string>
#include <stdexcept>


/* ==== Public Functions ==== */


void Application::Initialize() {
    m_window = InitWindow();
    m_instance = InitInstance();
    m_surface = InitSurface();
    m_adapter = InitAdapter();
    m_device = InitDevice();
    m_queue = InitQueue();
}

bool Application::IsRunning() {
    return !glfwWindowShouldClose(m_window);
}

void Application::MainLoop() {
    glfwPollEvents();

    m_encoder = CreateCommandEncoder();
    m_command = FinishCommandEncoder();
    SubmitCommandBuffer();

    WaitForQueueDone();
}

void Application::Terminate() {
    if (m_queue) {
        wgpuQueueRelease(m_queue);
        m_queue = nullptr;
    }
    if (m_device) {
        wgpuDeviceRelease(m_device);
        m_device = nullptr;
    }
    if (m_adapter) {
        wgpuAdapterRelease(m_adapter);
        m_adapter = nullptr;
    }
    if (m_surface) {
        wgpuSurfaceRelease(m_surface);
        m_surface = nullptr;
    }
    if (m_instance) {
        wgpuInstanceRelease(m_instance);
        m_instance = nullptr;
    }
    glfwTerminate();
    m_window = nullptr;
}


/* ==== Private Functions ==== */


/* ---- Initialization Functions ---- */


GLFWwindow* Application::InitWindow() {
    /* Initialize GLFW */

    int success = glfwInit();
    CheckGlfwInit(success);

    /* Initialize GLFW window hints */

    // Tell GLFW to not create an OpenGL context (by default)
    // WGPU manages its own connection to the GPU
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    /* Initialize GLFW window */

    GLFWwindow* window = glfwCreateWindow(800, 600, "Geodesix", nullptr, nullptr);
    CheckGlfwWindow(window);

    return window;
}

WGPUInstance Application::InitInstance() {
    WGPUInstance instance = wgpuCreateInstance(nullptr);
    CheckWgpuInstance(instance);

    return instance;
}

WGPUSurface Application::InitSurface() {
    WGPUSurface surface = glfwCreateWindowWGPUSurface(m_instance, m_window);
    CheckWgpuSurface(surface);

    return surface;
}

WGPUAdapter Application::InitAdapter() {
    WGPURequestAdapterOptions adapterOpts = {};
    adapterOpts.nextInChain = nullptr;
    adapterOpts.compatibleSurface = m_surface;
    WGPUAdapter adapter = requestAdapterSync(m_instance, &adapterOpts);
    CheckWgpuAdapter(adapter);

    return adapter;
}

WGPUDevice Application::InitDevice() {
    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = nullptr;
    deviceDesc.label = toWgpuStringView("Device");
    // We do not require any specific feature
    deviceDesc.requiredFeatureCount = 0;
    // We do not require any specific limit
    deviceDesc.requiredLimits = nullptr;
    deviceDesc.defaultQueue.nextInChain = nullptr;

    WGPUDevice device = requestDeviceSync(m_instance, m_adapter, &deviceDesc);
    CheckWgpuDevice(device);

    return device;
}

WGPUQueue Application::InitQueue() {
    WGPUQueue queue = wgpuDeviceGetQueue(m_device);
    CheckWgpuQueue(queue);

    return queue;
}


/* ---- Main Loop Functions ---- */


WGPUCommandEncoder Application::CreateCommandEncoder() {
    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.label = toWgpuStringView("Command Encoder");
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device, &encoderDesc);
    CheckWgpuCommandEncoder(encoder);

    return encoder;
}

WGPUCommandBuffer Application::FinishCommandEncoder() {
    WGPUCommandBufferDescriptor commandDescriptor = {};
    commandDescriptor.label = toWgpuStringView("Command Buffer");

    WGPUCommandBuffer command = wgpuCommandEncoderFinish(m_encoder, &commandDescriptor);
    CheckWgpuCommandBuffer(command);

    // Release the encoder after it is finished
    wgpuCommandEncoderRelease(m_encoder);
    m_encoder = nullptr;

    return command;
}

void Application::SubmitCommandBuffer() {
    wgpuQueueSubmit(m_queue, 1, &m_command);
    // Release the command buffer after it is finished
    wgpuCommandBufferRelease(m_command);
    m_command = nullptr;
}

void Application::WaitForQueueDone() {
    auto onQueuedWorkDone = [](
        WGPUQueueWorkDoneStatus status,
        void* userdata1,
        void* // userdata2: not needed for this callback
    ) {
        if (status != WGPUQueueWorkDoneStatus_Success) {
            std::cerr << "wgpuQueueOnSubmittedWorkDone failed!" << "\n";
        }

        bool& workDone = *reinterpret_cast<bool*>(userdata1);
        workDone = true;
    };

    bool workDone = false;

    WGPUQueueWorkDoneCallbackInfo callbackInfo = {};
    callbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
    callbackInfo.callback = onQueuedWorkDone;
    callbackInfo.userdata1 = &workDone;

    // Add the async operation to the queue
    wgpuQueueOnSubmittedWorkDone(m_queue, callbackInfo);

    // Check for pending async operations
    wgpuInstanceProcessEvents(m_instance);
    while (!workDone) {
        wgpuInstanceProcessEvents(m_instance);
    }
}


/* ---- Status & error check functions ---- */


void Application::CheckGlfwInit(int success) {
    if (success != GLFW_TRUE) {
        const char* description;
        glfwGetError(&description);

        std::string glfwError = description ? description : "Unknown";
        
        std::string errorMessage = "Failed to initialize GLFW! Error: " + glfwError;

        throw std::runtime_error(errorMessage);
    }
}

void Application::CheckGlfwWindow(GLFWwindow* window) {
    if (!window) {
        const char* description;
        glfwGetError(&description);

        std::string glfwError = description ? description : "Unknown";

        std::string errorMessage = "Failed to create GLFW window! Error: " + glfwError;
        
        throw std::runtime_error(errorMessage);
    }
}

void Application::CheckWgpuInstance(WGPUInstance instance) {
    if (!instance) {
        std::string errorMessage = "Failed to create WGPU instance!";

        throw std::runtime_error(errorMessage);
    }
}

void Application::CheckWgpuSurface(WGPUSurface surface) {
    if (!surface) {
        std::string errorMessage = "Failed to create WGPU surface!";

        throw std::runtime_error(errorMessage);
    }
}

void Application::CheckWgpuAdapter(WGPUAdapter adapter) {
    if (!adapter) {
        std::string errorMessage = "Failed to find a WGPU adapter!";

        throw std::runtime_error(errorMessage);
    }
}

void Application::CheckWgpuDevice(WGPUDevice device) {
    if (!device) {
        std::string errorMessage = "Failed to find a WGPU device!";

        throw std::runtime_error(errorMessage);
    }
}

void Application::CheckWgpuQueue(WGPUQueue queue) {
    if (!queue) {
        std::string errorMessage = "Failed to get the WGPU command queue!";

        throw std::runtime_error(errorMessage);
    }
}

void Application::CheckWgpuCommandEncoder(WGPUCommandEncoder encoder) {
    if (!encoder) {
        std::string errorMessage = "Failed to get the command encoder!";

        throw std::runtime_error(errorMessage);
    }
}

void Application::CheckWgpuCommandBuffer(WGPUCommandBuffer command) {
    if (!command) {
        std::string errorMessage = "Failed to finish the command buffer!";

        throw std::runtime_error(errorMessage);
    }
}
