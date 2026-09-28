#pragma once


#include <iostream>
#include <vector>
#include <string>

#include "header_inclusions/vulkan.hpp"

#include <glm/glm.hpp>


struct ExrImage {
    std::vector<glm::vec4> pixelData;
    uint32_t width = 0;
    uint32_t height = 0;
    vk::DeviceSize imageSize;
};

ExrImage LoadExr(const std::string& filePath);
