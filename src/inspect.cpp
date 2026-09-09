#include <iostream>
#include <webgpu/webgpu.h>

#include "inspect.h"


void inspectAdapter(WGPUAdapter adapter) {
    WGPUAdapterInfo info = {};
    wgpuAdapterGetInfo(adapter, &info);

    // Check for possible `nullptr` data before printing them to prevent a segmentation fault
    std::string vendor, architecture, device, description;
    if (info.vendor.data && info.vendor.length > 0) {
        vendor = info.vendor.data;
    } else {
        vendor = "(none)";
    }
    if (info.architecture.data && info.architecture.length > 0) {
        architecture = info.architecture.data;
    } else {
        architecture = "(none)";
    }
    if (info.device.data && info.device.length > 1) {
        device = info.device.data;
    } else {
        device = "(none)";
    }
    if (info.description.data && info.description.length > 0) {
        description = info.description.data;
    } else {
        description = "(none)";
    }

    std::cout << "\nAdapter Info" << std::endl;
    std::cout << "------------" << std::endl;
    std::cout << "Vendor:       " << vendor << std::endl;
    std::cout << "Architecture: " << architecture << std::endl;
    std::cout << "Device:       " << device << std::endl;
    std::cout << "Description:  " << description << std::endl;

    WGPULimits limits = {};
    wgpuAdapterGetLimits(adapter, &limits);

    std::cout << "\nAdapter Limits" << std::endl;
    std::cout << "--------------" << std::endl;
    std::cout << "Max Texture Dimension 2D: " << limits.maxTextureDimension2D << std::endl;
    std::cout << "Max Storage Buffer Size:  " << limits.maxStorageBufferBindingSize << std::endl;
}


void inspectDevice(WGPUDevice device) {
    WGPULimits limits = {};
    wgpuDeviceGetLimits(device, &limits);

    std::cout << "\nDevice Limits" << std::endl;
    std::cout << "---------------" << std::endl;
}
