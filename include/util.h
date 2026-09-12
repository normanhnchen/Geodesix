#pragma once

#include <webgpu/webgpu.h>
#include <iostream>
#include <string_view>


std::string_view toStdStringView(WGPUStringView wgpuStringView);
void sleepForMilliseconds(unsigned int milliseconds);
WGPUStringView toWgpuStringView(std::string_view stdStringView);
