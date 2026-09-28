#ifndef COMPUTE_BINDINGS_GLSL
#define COMPUTE_BINDINGS_GLSL


layout (binding = 0) uniform ParameterUBO {
    float deltaTime;
} ubo;

layout(binding = 1, rgba32f) uniform writeonly image2D computeStorageImage;

layout(binding = 2) uniform sampler2D envMap;

#endif