cmake_minimum_required(VERSION 3.20)
project(Geodesix)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    webgpu
    GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
    GIT_TAG main
)
FetchContent_MakeAvailable(webgpu)

FetchContent_Declare(
    glfw3webgpu
    GIT_REPOSITORY https://github.com/eliemichel/glfw3webgpu
    GIT_TAG main
)
FetchContent_MakeAvailable(glfw3webgpu)

add_executable(Geodesix
    src/main.cpp
    src/request.cpp
    src/inspect.cpp
)

target_include_directories(Geodesix PRIVATE include)

target_link_libraries(Geodesix PRIVATE glfw webgpu glfw3webgpu)

target_copy_webgpu_binaries(Geodesix)