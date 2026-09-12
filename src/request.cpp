#include <cassert>
#include <chrono>
#include <thread>
#include <iostream>
#include <webgpu/webgpu.h>

#include "request.h"
#include "util.h"


/*
 * Utility function to get a WebGPU adapter
 * Adapted from LearnWebGPU-Code (MIT License)
 * See THIRD_PARTY_NOTICES.md#learnwebgpu-code
 */
WGPUAdapter requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const * options) {
    struct UserData {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    // The callback must be a non-capturing (the brackets [] are empty) so
    // that it behaves like a regular C function which
    // wgpuInstanceRequestAdapter expects
    auto onAdapterRequestEnded = [](
        WGPURequestAdapterStatus status,
        WGPUAdapter adapter,
        WGPUStringView message,
        void* userData1,
        void* // userData2: not needed for this callback
    ) {
        // In a C function, we input userData1 as void*, so
        // reinterpret userData1 as a UserData object when called below
        UserData& userData = *reinterpret_cast<UserData*>(userData1);
        if (status == WGPURequestAdapterStatus_Success) {
            userData.adapter = adapter;
        } else {
            std::cout << "Could not get WebGPU adapter: " << message.data << std::endl;
        }
        userData.requestEnded = true;
    };

    WGPURequestAdapterCallbackInfo callbackInfo = {};
    callbackInfo.nextInChain = nullptr;
    callbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
    callbackInfo.callback = onAdapterRequestEnded;
    callbackInfo.userdata1 = &userData;
    callbackInfo.userdata2 = nullptr;

    wgpuInstanceRequestAdapter(instance, options, callbackInfo);

    // Check for pending async operations
    wgpuInstanceProcessEvents(instance);

    while (!userData.requestEnded) {
        // Sleep to avoid requesting too often
        sleepForMilliseconds(200);

        // Check for pending async operations
        wgpuInstanceProcessEvents(instance);
    }

    return userData.adapter;
}


/*
 * Utility function to get a WebGPU device
 * Adapted from LearnWebGPU-Code (MIT License)
 * See THIRD_PARTY_NOTICES.md#learnwebgpu-code
 */
WGPUDevice requestDeviceSync(WGPUInstance instance, WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor) {
    struct UserData {
        WGPUDevice device = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    // The callback must be a non-capturing (the brackets [] are empty) so
    // that it behaves like a regular C function which
    // wgpuInstanceRequestAdapter expects
    auto onDeviceRequestEnded = [](
        WGPURequestDeviceStatus status,
        WGPUDevice device,
        WGPUStringView message,
        void* userData1,
        void* // userData2: not needed for this callback
    ) {
        // In a C function, we input userData1 as void*, so
        // reinterpret userData1 as a UserData object when called below
        UserData& userData = *reinterpret_cast<UserData*>(userData1);
        if (status == WGPURequestDeviceStatus_Success) {
            userData.device = device;
        } else {
            std::cout << "Could not get WebGPU device: " << message.data << std::endl;
        }
        userData.requestEnded = true;
    };

    WGPURequestDeviceCallbackInfo callbackInfo = {};
    callbackInfo.nextInChain = nullptr;
    callbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
    callbackInfo.callback = onDeviceRequestEnded;
    callbackInfo.userdata1 = &userData;
    callbackInfo.userdata2 = nullptr;

    wgpuAdapterRequestDevice(adapter, descriptor, callbackInfo);

    // Check for pending async operations
    wgpuInstanceProcessEvents(instance);
    while (!userData.requestEnded) {
        // Sleep to avoid requesting too often
        sleepForMilliseconds(200);

        // Check for pending async operations
        wgpuInstanceProcessEvents(instance);
    }

    return userData.device;
}
