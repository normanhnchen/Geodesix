#include <stdexcept>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <ImfRgbaFile.h>
#include <ImfRgba.h>
#include <ImfArray.h>

#include "exr_loading.hpp"


ExrImage LoadExr(const std::string& filePath) {
    ExrImage img;
    try {
        Imf::RgbaInputFile file(filePath.c_str());
        Imath::Box2i dw = file.dataWindow();

        img.width = dw.max.x - dw.min.x + 1;
        img.height = dw.max.y - dw.min.y + 1;

        // Match OpenEXR's internal 16-bit struct
        // NOTE: EXR files commonly have 16-bit color depth
        std::vector<Imf::Rgba> tempPixels(img.width * img.height);

        // Offset the base pointer backward to map OpenEXR's absolute canvas coordinates 
        // directly to our zero-indexed and memory-optimized buffer
        Imf::Rgba* basePtr = tempPixels.data() - dw.min.x - (dw.min.y * static_cast<int>(img.width));
        file.setFrameBuffer(basePtr, 1, img.width);

        file.readPixels(dw.min.y, dw.max.y);
        
        img.pixelData.resize(img.width * img.height);
        img.imageSize = img.pixelData.size() * sizeof(glm::vec4); // 16 bytes per pixel

        for (size_t i = 0; i < tempPixels.size(); ++i) {
            /* Cast from 16-bit half-floats to 32-bit floats */
            img.pixelData[i].r = static_cast<float>(tempPixels[i].r);
            img.pixelData[i].g = static_cast<float>(tempPixels[i].g);
            img.pixelData[i].b = static_cast<float>(tempPixels[i].b);
            img.pixelData[i].a = static_cast<float>(tempPixels[i].a);
        }
    } catch (const std::exception &e) {
        throw std::runtime_error("Error reading EXR file: " + std::string(e.what()));
    }

    return img;
}
