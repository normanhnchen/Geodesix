#include <iostream>
#include <webgpu/webgpu.h>

#include "inspect.h"


// Adapted from LearnWebGPU-Code by Élie Michel (https://github.com/eliemichel/LearnWebGPU-Code)
// MIT License
std::string_view toStdStringView(WGPUStringView wgpuStringView) {
    if (wgpuStringView.data == nullptr) {
        return std::string_view();
    } else {
        if (wgpuStringView.length == WGPU_STRLEN) {
            return std::string_view(wgpuStringView.data);
        } else {
            return std::string_view(wgpuStringView.data, wgpuStringView.length);
        }
    }
}


// Adapted from LearnWebGPU-Code by Élie Michel (https://github.com/eliemichel/LearnWebGPU-Code)
// MIT License
void inspectAdapter(WGPUAdapter adapter) {
	WGPULimits supportedLimits = {};
	supportedLimits.nextInChain = nullptr;
	
	bool success = wgpuAdapterGetLimits(adapter, &supportedLimits) == WGPUStatus_Success;
	
	if (success) {
		std::cout << "Adapter limits:" << std::endl;
		std::cout << " - maxTextureDimension1D: " << supportedLimits.maxTextureDimension1D << std::endl;
		std::cout << " - maxTextureDimension2D: " << supportedLimits.maxTextureDimension2D << std::endl;
		std::cout << " - maxTextureDimension3D: " << supportedLimits.maxTextureDimension3D << std::endl;
		std::cout << " - maxTextureArrayLayers: " << supportedLimits.maxTextureArrayLayers << std::endl;
	}
	WGPUSupportedFeatures features;
	
	// Get adapter features. This may allocate memory that we must later free with wgpuSupportedFeaturesFreeMembers()
	wgpuAdapterGetFeatures(adapter, &features);
	
	std::cout << "Adapter features:" << std::endl;
	std::cout << std::hex; // Write integers as hexadecimal to ease comparison with webgpu.h literals
	for (size_t i = 0; i < features.featureCount; ++i) {
		std::cout << " - 0x" << features.features[i] << std::endl;
	}
	std::cout << std::dec; // Restore decimal numbers
	
	// Free the memory that had potentially been allocated by wgpuAdapterGetFeatures()
	wgpuSupportedFeaturesFreeMembers(features);
	// One shall no longer use features beyond this line.
	WGPUAdapterInfo properties;
	properties.nextInChain = nullptr;
	wgpuAdapterGetInfo(adapter, &properties);
	std::cout << "Adapter properties:" << std::endl;
	std::cout << " - vendorID: " << properties.vendorID << std::endl;
	std::cout << " - vendorName: " << toStdStringView(properties.vendor) << std::endl;
	std::cout << " - architecture: " << toStdStringView(properties.architecture) << std::endl;
	std::cout << " - deviceID: " << properties.deviceID << std::endl;
	std::cout << " - name: " << toStdStringView(properties.device) << std::endl;
	std::cout << " - driverDescription: " << toStdStringView(properties.description) << std::endl;
	std::cout << std::hex;
	std::cout << " - adapterType: 0x" << properties.adapterType << std::endl;
	std::cout << " - backendType: 0x" << properties.backendType << std::endl;
	std::cout << std::dec; // Restore decimal numbers
	wgpuAdapterInfoFreeMembers(properties);
}

// Adapted from LearnWebGPU-Code by Élie Michel (https://github.com/eliemichel/LearnWebGPU-Code)
// MIT License
void inspectDevice(WGPUDevice device) {
    
    WGPUSupportedFeatures features = {};
    wgpuDeviceGetFeatures(device, &features);
    std::cout << "Device features:" << std::endl;
    // Feature IDs are conventionally shown in hex
    std::cout << std::hex;
    for (size_t i = 0; i < features.featureCount; ++i) {
        std::cout << " - 0x" << features.features[i] << std::endl;
    }
    std::cout << std::dec;
    wgpuSupportedFeaturesFreeMembers(features);

    WGPULimits limits = {};
    bool success = wgpuDeviceGetLimits(device, &limits) == WGPUStatus_Success;

    if (success) {
        std::cout << "Device limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: " << limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: " << limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: " << limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: " << limits.maxTextureArrayLayers << std::endl;
    }
}
