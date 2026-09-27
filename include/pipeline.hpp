#pragma once


#include <iostream>
#include <vector>
#include <filesystem>

#include "header_inclusions/vulkan.hpp"

#include "vulkan_context.hpp"
#include "swap_chain.hpp"
#include "buffer_context.hpp"


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
#define TOPOLOGY_TRIANGLE_LIST
/** 
 * Draw triangles where every second and third vertex of every triangle are reused for the next
 * triangle's first two vertices.
 * 
 * vk::PrimitiveTopology::eTriangleStrip
 */
// #define TOPOLOGY_TRIANGLE_STRIP

const std::filesystem::path SHADER_SPIRV_DIR = CMAKE_SHADER_SPIRV_DIR;
const std::string SHADER_MAIN_VERT_PATH = std::string(SHADER_SPIRV_DIR / "main.vert.spv");
const std::string SHADER_MAIN_FRAG_PATH = std::string(SHADER_SPIRV_DIR / "main.frag.spv");
const std::string SHADER_MAIN_COMP_PATH = std::string(SHADER_SPIRV_DIR / "main.comp.spv");

/**
 * The entry point is the function where the shader starts executing. In GLSL, there can only be
 * one entry point per file (that being void main()). In other languages like Slang, there can be
 * multiple entry points where each could represent a shader stage. For GLSL, we default to "main"
 * for all shader modules.
 */
constexpr const char* SHADER_ENTRY_POINT = "main";

class Pipeline {
public:
    Pipeline(
        VulkanContext& vulkanContext,
        SwapChain& swapChain,
        BufferContext& bufferContext
    );

    void Init();

    const vk::raii::Pipeline& GetGraphicsPipeline() const;
    const vk::raii::PipelineLayout& GetGraphicsPipelineLayout() const;

    const vk::raii::Pipeline& GetComputePipeline() const;
    const vk::raii::PipelineLayout& GetComputePipelineLayout() const;

private:
    VulkanContext& m_vulkanContext;
    SwapChain& m_swapChain;
    BufferContext& m_bufferContext;

    vk::raii::PipelineLayout m_pipelineLayout = nullptr;
    vk::raii::Pipeline m_graphicsPipeline = nullptr;

    vk::raii::PipelineLayout m_computePipelineLayout = nullptr;
    vk::raii::Pipeline m_computePipeline = nullptr;

    void CreateGraphicsPipeline();
    void CreateComputePipeline();

    [[nodiscard]] vk::raii::ShaderModule CreateShaderModule(
        const std::vector<char>& code
    ) const;
};
