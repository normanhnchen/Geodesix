#pragma once


#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>


struct ExrImage {
    std::vector<glm::vec4> pixelData;
    uint32_t width = 0;
    uint32_t height = 0;
};

ExrImage LoadExr(const std::string& filePath);
