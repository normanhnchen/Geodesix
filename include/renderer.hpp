#pragma once


#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "window.hpp"
#include "vulkan_context.hpp"
#include "swap_chain.hpp"


/**
 * ---- Topology Modes ---
 * These macros are used in Appication::CreateGraphicsPipeline for the inputAssembly struct.
 * 
 * They represent how the geometry will be drawn from vertices sent to the GPU and the primitive.
 * 
 * NOTE: only one of the macros should be defined or else it might lead to unexpected behavior!
 * 
 * NOTE: for any mode other than eFill, a physical device feature must be enabled (see
 * VulkanContext::CreateLogicalDevice -> featureChain struct)
 */

/** 
 * Draw points from vertices.
 * 
 * vk::PrimitiveTopology::ePointList
 */
// #define TOPOLOGY_POINT_LIST
/** 
 * Draw lines between every two vertices (without reusing vertices).
 * 
 * vk::PrimitiveTopology::eLineList
 */
// #define TOPOLOGY_LINE_LIST
/** 
 * Draw lines where the end vertex of every line is used as the start vertex for the next line.
 * 
 * vk::PrimitiveTopology::eLineStrip
 */
// #define TOPOLOGY_LINE_STRIP
/** 
 * Draw triangles from every three vertices (without reusing vertices).
 * 
 * vk::PrimitiveTopology::eTriangleList
 */
// #define TOPOLOGY_TRIANGLE_LIST
/** 
 * Draw triangles where every second and third vertex of every triangle are reused for the next
 * triangle's first two vertices.
 * 
 * vk::PrimitiveTopology::eTriangleStrip
 */
#define TOPOLOGY_TRIANGLE_STRIP

const std::filesystem::path SHADER_SPIRV_DIR = CMAKE_SHADER_SPIRV_DIR;
const std::string SHADER_MAIN_VERT_PATH = std::string(SHADER_SPIRV_DIR / "main.vert.spv");
const std::string SHADER_MAIN_FRAG_PATH = std::string(SHADER_SPIRV_DIR / "main.frag.spv");

/**
 * The entry point is the function where the shader starts executing. In GLSL, there can only be
 * one entry point per file (that being void main()). In other languages like Slang, there can be
 * multiple entry points where each could represent a shader stage. For GLSL, we default to "main"
 * for all shader modules.
 */
constexpr const char* SHADER_ENTRY_POINT = "main";

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

class Renderer {
public:
    Renderer(
        Window& window,
        VulkanContext& vulkanContext,
        SwapChain& swapChain
    );

    void Init();

    void DrawFrame();

private:
    Window& m_window;
    VulkanContext& m_vulkanContext;
    SwapChain& m_swapChain;
    vk::raii::PipelineLayout m_pipelineLayout = nullptr;
    vk::raii::Pipeline m_graphicsPipeline = nullptr;
    vk::raii::CommandPool m_commandPool = nullptr;
    std::vector<vk::raii::CommandBuffer> m_commandBuffers;
    std::vector<vk::raii::Semaphore> m_presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_inFlightFences;

    uint32_t m_frameIndex = 0;

    static std::vector<char> ReadFile(const std::string &filePath);

    [[nodiscard]] vk::raii::ShaderModule CreateShaderModule(
        const std::vector<char>& code
    ) const;

    void CreateGraphicsPipeline();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void RecordCommandBuffer(uint32_t imageIndex);
    void TransitionImageLayout(
        uint32_t imageIndex,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask
    );

    void CreateSyncObjects();
};
