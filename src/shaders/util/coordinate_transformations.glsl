#ifndef UTIL_COORDINATE_TRANSFORMATIONS_GLSL
#define UTIL_COORDINATE_TRANSFORMATIONS_GLSL


vec3 CartesianToSpherical(vec3 cartesian) {
    float x = cartesian.x;
    float y = cartesian.y;
    float z = cartesian.z;

    float r = length(cartesian);
    float theta = (x == 0.0 && y == 0.0) ? 0.0: atan(y, x);
    float phi = asin(clamp(z/max(r, 1e-4), -1.0, 1.0));

    return vec3(r, theta, phi);
}

vec3 SphericalToCartesian(vec3 spherical) {
    float r = spherical.x;
    float theta = spherical.y;
    float phi = spherical.z;

    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    float x = r * cosTheta * cosPhi;
    float y = r * sinTheta * cosPhi;
    float z = r * sinPhi;

    return vec3(x, y, z);
}

vec2 SphericalToUv(vec3 spherical) {
    float theta = spherical.y;
    float phi = spherical.z;

    float u = theta / (2.0*PI) + 0.5;
    float v = phi / (PI) + 0.5;

    // NOTE: Vulkan's UV texture mapping has v flipped
    v = 1.0 - v;

    return vec2(u, v);
}

vec3 UvToSpherical(vec2 uv) {
    float u = uv.x;
    float v = uv.y;

    // Undo the Vulkan v-flip applied in SphericalToUv
    v = 1.0 - v;

    float theta = 2.0 * PI * (u - 0.5);
    float phi = PI * (v - 0.5);

    // UV carries no radial information; assume a unit sphere
    return vec3(1.0, theta, phi);
}

vec2 CartesianToUv(vec3 cartesian) {
    vec3 spherical = CartesianToSpherical(cartesian);
    vec2 uv = SphericalToUv(spherical);

    return uv;
}

vec3 UvToCartesian(vec2 uv) {
    vec3 spherical = UvToSpherical(uv);
    vec3 cartesian = SphericalToCartesian(spherical);

    return cartesian;
}

vec3 DirectionToSpherical(vec3 dir) {
    float x = dir.x;
    float y = dir.y;
    float z = dir.z;

    float theta = (x == 0.0 && y == 0.0) ? 0.0 : atan(y, x);
    float phi = asin(clamp(z, -1.0, 1.0));

    return vec3(1.0, theta, phi);
}

vec3 SphericalToDirection(vec3 spherical) {
    float theta = spherical.y;
    float phi = spherical.z;

    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    float x = cosTheta * cosPhi;
    float y = sinTheta * cosPhi;
    float z = sinPhi;

    return vec3(x, y, z);
}

vec2 DirectionToUv(vec3 dir) {
    vec3 spherical = DirectionToSpherical(dir);
    vec2 uv = SphericalToUv(spherical);

    return uv;
}

vec3 UvToDirection(vec2 uv) {
    vec3 spherical = UvToSpherical(uv);
    vec3 dir = SphericalToDirection(spherical);

    return dir;
}


#endif