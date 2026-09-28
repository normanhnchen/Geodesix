#ifndef COMPUTE_STRUCTS_GLSL
#define COMPUTE_STRUCTS_GLSL


struct Ray {
    vec3 origin;
    vec3 dir;
};

struct CameraRay {
    vec3 origin;
    vec3 dir;
};


#endif