#ifndef COMPUTE_BINDINGS_GLSL
#define COMPUTE_BINDINGS_GLSL


/*
 * ===========================================================================================
 * NOTE: Uniform buffer objects require the memory of its data to be aligned in a specific way.
 *
 * https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-resources-layout
 * ===========================================================================================
*/


layout(binding = 0) uniform CameraUbo {
    vec3 pos;
    float fov;
} camera;

layout(binding = 1) uniform ParameterUBO {
    float deltaTime;
    float time;
    vec2 pad1;
} parameters;

layout(binding = 2, rgba32f) uniform writeonly image2D computeStorageImage;

layout(binding = 3) uniform sampler2D envMap;

#endif