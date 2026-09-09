#include <cassert>
#include <iostream>
#include <string_view>
#include <webgpu/webgpu.h>

#include "request.h"


WGPUAdapter requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const * options) {
    struct UserData {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void * pUserData1, void * pUserData2) {
        UserData& userData = *reinterpret_cast<UserData*>(pUserData1);
        if (status == WGPURequestAdapterStatus_Success) {
            userData.adapter = adapter;
        } else {
            std::cout << "Could not get WebGPU adapter: " << message.data << std::endl;
        }
        userData.requestEnded = true;
    };

    WGPURequestAdapterCallbackInfo callbackInfo = {};
    callbackInfo.callback = onAdapterRequestEnded;
    callbackInfo.userdata1 = &userData;

    wgpuInstanceRequestAdapter(
        instance,
        options,
        callbackInfo
    );

    assert(userData.requestEnded);

    return userData.adapter;
}


WGPUDevice requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor) {
    struct UserData {
        WGPUDevice device = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void * pUserData1, void * pUserData2) {
        UserData& userData = *reinterpret_cast<UserData*>(pUserData1);
        if (status == WGPURequestDeviceStatus_Success) {
            userData.device = device;
        } else {
            std::cout << "Could not get WebGPU device: " << message.data << std::endl;
        }
        userData.requestEnded = true;
    };

    WGPURequestDeviceCallbackInfo callbackInfo = {};
    callbackInfo.callback = onDeviceRequestEnded;
    callbackInfo.userdata1 = &userData;

    wgpuAdapterRequestDevice(
        adapter,
        descriptor,
        callbackInfo
    );

    assert(userData.requestEnded);

    return userData.device;
}
