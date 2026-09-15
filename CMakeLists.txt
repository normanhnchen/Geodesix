cmake_minimum_required(VERSION 3.28)
project(Geodesix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(Geodesix
    src/main.cpp
    src/application.cpp
)

target_link_libraries(Geodesix PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
)

target_include_directories(Geodesix PRIVATE include)
