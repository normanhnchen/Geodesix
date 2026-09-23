#ifndef UBOS_GLSL
#define UBOS_GLSL


/*
 * ===========================================================================================
 * NOTE: Uniform buffer objects require the memory of its data to be aligned in a specific way.
 *
 * https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-resources-layout
* ===========================================================================================
*/


/* ---- main.vert ---- */


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;


/* ---- main.frag ---- */





#endif