#pragma once

#include <webgpu/webgpu.h>
#include <iostream>
#include <string_view>


std::string_view toStdStringView(WGPUStringView wgpuStringView);
void sleepForMilliseconds(unsigned int milliseconds);
