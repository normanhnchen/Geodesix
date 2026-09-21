cmake_minimum_required(VERSION 3.28)
project(Geodesix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SHADER_DIR "${PROJECT_SOURCE_DIR}/src/shaders")
set(PREPROCESSED_DIR "${CMAKE_BINARY_DIR}/shaders/preprocessed")
set(SPIR_V_DIR "${CMAKE_BINARY_DIR}/shaders")
set(SHADER_PREPROCESS_SCRIPT "${PROJECT_SOURCE_DIR}/scripts/shader_preprocess.py")
find_package(Python3 COMPONENTS Interpreter REQUIRED)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(Geodesix
    src/main.cpp
    src/application.cpp
    src/window.cpp
    src/vulkan_context.cpp
    src/swap_chain.cpp
    src/renderer.cpp
    src/pipeline.cpp
)

target_link_libraries(Geodesix PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
)

# Locate the glslangValidator (the GLSL -> SPIR-V compiler that comes with the Vulkan SDK)
# Adapted from the Vulkan Guide (https://github.com/vblanco20-1/vulkan-guide)
# Author: Victor Blanco (2020)
# License: MIT License
find_program(GLSL_VALIDATOR glslangValidator
    HINTS
        /usr/bin
        /usr/local/bin
        $ENV{VULKAN_SDK}/Bin/
        $ENV{VULKAN_SDK}/Bin32/
)

# Locate all shader files in the repository
# Adapted from the Vulkan Guide (https://github.com/vblanco20-1/vulkan-guide)
# Author: Victor Blanco (2020)
# License: MIT License
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_DIR}/*.frag"
    "${SHADER_DIR}/*.vert"
    "${SHADER_DIR}/*.comp"
    )

# Make sure the output directories exist
file(MAKE_DIRECTORY ${PREPROCESSED_DIR} ${SPIR_V_DIR})

# Edits to included GLSL files trigger a rebuild
file(GLOB_RECURSE GLSL_INCLUDE_FILES
    "${SHADER_DIR}/*.glsl"
    "${SHADER_DIR}/*.frag"
    "${SHADER_DIR}/*.vert"
    "${SHADER_DIR}/*.comp"
    )

# Loop through every located shader file
# Adapted from the Vulkan Guide (https://github.com/vblanco20-1/vulkan-guide)
# Author: Victor Blanco (2020)
# License: MIT License
foreach(GLSL ${GLSL_SOURCE_FILES})
    message(STATUS "Building shader...")
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SPIR_V_DIR}/${FILE_NAME}.spv")
    set(PREPROCESSED "${PREPROCESSED_DIR}/${FILE_NAME}")
    message(STATUS ${GLSL})

    # Parse shader files with the python shader preprocessor script
    add_custom_command(
        OUTPUT ${PREPROCESSED}
        COMMAND Python3::Interpreter ${SHADER_PREPROCESS_SCRIPT}
            # NOTE: must match the exact script flags in the python script
            --source ${GLSL}
            --dest ${PREPROCESSED}
        DEPENDS ${GLSL} ${GLSL_INCLUDE_FILES} ${SHADER_PREPROCESS_SCRIPT}
        COMMENT "Preprocessing includes for ${FILE_NAME}")

    # Execute glslang to compile the preprocessed shader files
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${PREPROCESSED} -o ${SPIRV}
        DEPENDS ${PREPROCESSED})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(Shaders ALL DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(Geodesix Shaders)

# Define the SPIR-V compiled binaries for the CPU side to use
target_compile_definitions(Geodesix PRIVATE CMAKE_SHADER_SPIRV_DIR="${SPIR_V_DIR}")

target_include_directories(Geodesix PRIVATE include)
