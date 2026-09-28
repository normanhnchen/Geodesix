#include <iostream>
#include <string>

#include "header_inclusions/vulkan.hpp"

#include "pipeline.hpp"
#include "vk_util.hpp"
#include "buffer_data.hpp"


Pipeline::Pipeline(
    VulkanContext& vulkanContext,
    SwapChain& swapChain,
    BufferContext& bufferContext
)
    : m_vulkanContext(vulkanContext),
    m_swapChain(swapChain),
    m_bufferContext(bufferContext) {
}

void Pipeline::Init() {
    CreateGraphicsPipeline();
    CreateComputePipeline();
}

const vk::raii::Pipeline& Pipeline::GetGraphicsPipeline() const {
    return m_graphicsPipeline;
}

const vk::raii::PipelineLayout& Pipeline::GetGraphicsPipelineLayout() const {
    return m_pipelineLayout;
}

const vk::raii::Pipeline& Pipeline::GetComputePipeline() const {
    return m_computePipeline;
}

const vk::raii::PipelineLayout& Pipeline::GetComputePipelineLayout() const {
    return m_computePipelineLayout;
}

/**
 * @brief Create the graphics pipeline for rendering.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/00_Introduction.html
 */
void Pipeline::CreateGraphicsPipeline() {
    vk::SurfaceFormatKHR swapChainSurfaceFormat = m_swapChain.GetSurfaceFormat();
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    const vk::raii::DescriptorSetLayout& graphicsDescriptorSetLayout = m_bufferContext.GetGraphicsDescriptorSetLayout();

    auto shaderCodeMainVert = vk_util::ReadFile(SHADER_MAIN_VERT_PATH);
    auto shaderCodeMainFrag = vk_util::ReadFile(SHADER_MAIN_FRAG_PATH);

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

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };
    
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
        // Flipping the y-axis (see CommandContext::RecordCommandBuffer, viewport struct) causes
        // the vertices are being drawn in counter-clockwise order
        // NOTE: only flip the direction 
        .frontFace = vk::FrontFace::eCounterClockwise,
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
        .setLayoutCount = 1,
        .pSetLayouts = &*graphicsDescriptorSetLayout,
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

void Pipeline::CreateComputePipeline() {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    const vk::raii::DescriptorSetLayout& computeDescriptorSetLayout = m_bufferContext.GetComputeDescriptorSetLayout();

    auto shaderCodeMainComp = vk_util::ReadFile(SHADER_MAIN_COMP_PATH);
    vk::raii::ShaderModule shaderModule = CreateShaderModule(shaderCodeMainComp);

    vk::PipelineShaderStageCreateInfo computeShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eCompute,
        .module = shaderModule,
        .pName = SHADER_ENTRY_POINT
    };
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = 1,
        .pSetLayouts = &*computeDescriptorSetLayout
    };
    m_computePipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);
    vk::ComputePipelineCreateInfo pipelineInfo{
        .stage = computeShaderStageInfo,
        .layout = *m_computePipelineLayout
    };
    m_computePipeline = vk::raii::Pipeline(device, nullptr, pipelineInfo);
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
