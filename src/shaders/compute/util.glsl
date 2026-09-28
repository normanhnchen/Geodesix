#ifndef COMPUTE_UTIL_GLSL
#define COMPUTE_UTIL_GLSL


vec2 DirToUv(vec3 dir) {
    float u = atan(dir.y, dir.x) / (2.0 * PI) + 0.5;
    float v = 0.5 - asin(clamp(dir.z, -1.0, 1.0)) / PI;
    
    return vec2(u, v);
}

vec2 UvToSpherical(vec2 uv) {
    float phi = (uv.x - 0.5) * (2.0 * PI);
    float theta = (0.5 - uv.y) * PI;

    return vec2(phi, theta);
}

vec3 SphericalToDir(float phi, float theta) {
    float x = cos(theta) * cos(phi);
    float y = cos(theta) * sin(phi);
    float z = sin(theta);

    return vec3(x, y, z);
}

vec3 UvToDir(vec2 uv) {
    vec2 spherical = UvToSpherical(uv);
    float phi = spherical.x;
    float theta = spherical.y;

    vec3 dir = SphericalToDir(phi, theta);

    return dir;
}

vec3 SampleEquirectangular(sampler2D tex, vec3 dir) {
    vec2 uv = DirToUv(dir);
    return textureLod(tex, uv, 0.0).rgb;
}


#endif