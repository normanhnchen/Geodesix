#ifndef UTIL_TEXTURE_SAMPLING_GLSL
#define UTIL_TEXTURE_SAMPLING_GLSL


#include "src/shaders/util/coordinate_transformations.glsl"


vec3 SampleEquirectangularTexture(sampler2D tex, vec3 dir) {
    vec2 uv = DirectionToUv(dir);
    // Use `textureLod` with no level-of-detail (LOD) for predictable texture quality
    return textureLod(tex, uv, 0.0).rgb;
}


#endif