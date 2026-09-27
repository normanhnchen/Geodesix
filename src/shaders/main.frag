#version 460


#include "src/shaders/constants.glsl"


layout(binding = 1) uniform sampler2D computeStorageTexture;

layout(location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;


void main() {
    outColor = texture(computeStorageTexture, texCoords);
}
