#include <webgpu/webgpu.h>
#include <iostream>
#include <chrono>
#include <thread>

#include "util.h"


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
WGPUStringView toWgpuStringView(std::string_view stdStringView) {
	return {
        stdStringView.data(),
        stdStringView.size()
    };
}


// Adapted from LearnWebGPU-Code by Élie Michel (https://github.com/eliemichel/LearnWebGPU-Code)
// MIT License
void sleepForMilliseconds(unsigned int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
