#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "pipeline.hpp"


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

Pipeline::Pipeline(VulkanContext& vulkanContext, SwapChain& swapChain)
    : m_vulkanContext(vulkanContext), m_swapChain(swapChain) {
}

void Pipeline::Init() {
    Create();
}

const vk::raii::Pipeline& Pipeline::GetGraphicsPipeline() const {
    return m_graphicsPipeline;
}

/**
 * @brief Create the graphics pipeline for rendering.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/00_Introduction.html
 */
void Pipeline::Create() {
    vk::SurfaceFormatKHR swapChainSurfaceFormat = m_swapChain.GetSurfaceFormat();

    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    auto shaderCodeMainVert = ReadFile(SHADER_MAIN_VERT_PATH);
    auto shaderCodeMainFrag = ReadFile(SHADER_MAIN_FRAG_PATH);

    vk::raii::ShaderModule shaderModuleMainVert = CreateShaderModule(shaderCodeMainVert);
    vk::raii::ShaderModule shaderModuleMainFrag = CreateShaderModule(shaderCodeMainFrag);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shaderModuleMainVert,
        .pName = SHADER_ENTRY_POINT
    };
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shaderModuleMainFrag,
        .pName = SHADER_ENTRY_POINT
    };
    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo
    };

    std::vector<vk::DynamicState> dynamicStates = {
        /* Allow these states to be updated during runtime */
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
#ifdef TOPOLOGY_POINT_LIST
        .topology = vk::PrimitiveTopology::ePointList,
#endif
#ifdef TOPOLOGY_LINE_LIST
        .topology = vk::PrimitiveTopology::eLineList,
#endif
#ifdef TOPOLOGY_LINE_STRIP
        .topology = vk::PrimitiveTopology::eLineStrip,
#endif
#ifdef TOPOLOGY_TRIANGLE_LIST
        .topology = vk::PrimitiveTopology::eTriangleList,
#endif
#ifdef TOPOLOGY_TRIANGLE_STRIP
        .topology = vk::PrimitiveTopology::eTriangleStrip
#endif
    };
    vk::PipelineViewportStateCreateInfo viewportState{
        .viewportCount = 1,
        .scissorCount = 1
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
#ifdef POLYGON_FILL
        .polygonMode = vk::PolygonMode::eFill,
#endif
#ifdef POLYGON_LINE
        .polygonMode = vk::PolygonMode::eLine,
#endif
#ifdef POLYGON_POINT
        .polygonMode = vk::PolygonMode::ePoint,
#endif
        .cullMode = vk::CullModeFlagBits::eNone,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{
        /**
         * Disable multisampling.
         * 
         * NOTE: enabling this feature in the future will require a GPU feature.
         */
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        /**
         * Disable color blending.
         */
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0
    };

    m_pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapChainSurfaceFormat.format
    };
    vk::GraphicsPipelineCreateInfo graphicsRenderingCreateInfo{
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = m_pipelineLayout,
        // Set to nullptr because the render passes will be dynamic
        .renderPass = nullptr
    };

    vk::StructureChain<
        vk::GraphicsPipelineCreateInfo,
        vk::PipelineRenderingCreateInfo
    > pipelineCreateInfoChain = {
        graphicsRenderingCreateInfo,
        pipelineRenderingCreateInfo
    };

    m_graphicsPipeline = vk::raii::Pipeline(
        device,
        nullptr,
        pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>()
    );
}

/**
 * @brief Reads the bytes of a specified file.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/01_Shader_modules.html
 */
std::vector<char> Pipeline::ReadFile(const std::string &filePath) {
    // Open the file at the end (ate) in binary mode
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file!");
    }
    // Get the exact number of bytes the file has and allocate it to a buffer
    std::vector<char> buffer(file.tellg());
    // Reset the read cursor to beginning
    file.seekg(0, std::ios::beg);
    // Read the raw bytes
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();
    return buffer;
}

/**
 * @brief Create a Vulkan shader module from code (in bytes).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/01_Shader_modules.html
 */
[[nodiscard]] vk::raii::ShaderModule Pipeline::CreateShaderModule(
    const std::vector<char>& code
) const {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    vk::ShaderModuleCreateInfo createInfo{
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t *>(code.data())
    };
    vk::raii::ShaderModule shaderModule{device, createInfo};

    return shaderModule;
}
