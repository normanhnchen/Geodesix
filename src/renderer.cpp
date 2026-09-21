#include "renderer.hpp"


Renderer::Renderer(
    Window& window,
    VulkanContext& vulkanContext,
    SwapChain& swapChain
)
    : m_window(window), m_vulkanContext(vulkanContext), m_swapChain(swapChain) {
}

void Renderer::Init() {
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateCommandBuffer();
    CreateSyncObjects();
}

/**
 * @brief Draw a frame.
 * 
 * Called from the main loop.
 * 
 * A Vulkan sephamore is a synchronization object used to order GPU queue operations (GPU -> GPU).
 * 
 * A Vulkan fence is a synchronization object used to order CPU queue operations (CPU -> CPU).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html
 */
void Renderer::DrawFrame() {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    const vk::raii::Queue& queue = m_vulkanContext.GetQueue();
    const vk::raii::SwapchainKHR& swapChain = m_swapChain.GetNative();
    std::vector<vk::Image> swapChainImages = m_swapChain.GetImages();
    vk::Extent2D swapChainExtent = m_swapChain.GetExtent();
    const std::vector<vk::raii::ImageView>& swapChainImageViews = m_swapChain.GetImageViews();

    /*  Wait until the previous frame is finished */
    auto fenceResult = device.waitForFences(
        *m_inFlightFences[m_frameIndex],
        vk::True,
        UINT64_MAX // Timeout
    );
    if (fenceResult != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence!");
    }

    auto imageIndexOpt = m_swapChain.AcquireNextImageIndex(
        m_presentCompleteSemaphores[m_frameIndex]
    );

    if (imageIndexOpt == std::nullopt) {
        /* The swap chain is incompatible with the surface and can no longer be used to render */
        return;
    }

    uint32_t imageIndex = *imageIndexOpt;

    // Prevent a deadlock by resetting the fence only when we know it will be submitting work later
    // (we acquired the next image and it passed all of the error checks)
    device.resetFences(*m_inFlightFences[m_frameIndex]);

    RecordCommandBuffer(imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask(
        vk::PipelineStageFlagBits ::eColorAttachmentOutput
    );
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_presentCompleteSemaphores[m_frameIndex],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*m_commandBuffers[m_frameIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*m_renderFinishedSemaphores[imageIndex]
    };

    queue.submit(submitInfo, *m_inFlightFences[m_frameIndex]);

    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_renderFinishedSemaphores[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &*swapChain,
        .pImageIndices = &imageIndex
    };

    auto result = queue.presentKHR(presentInfoKHR);

    if (
        // The swap chain is incompatible with the surface and can no longer be used to render
        (result == vk::Result::eSuboptimalKHR) ||
        // The surface properties don't match anymore
        (result == vk::Result::eErrorOutOfDateKHR) ||
        m_window.Resized()
    ) {
        m_window.ResetResizedFlag();
        m_swapChain.Recreate();
    } else {
        // On any other error besides eSuccess, presentKHR throws an exception
        assert(result == vk::Result::eSuccess);
    }

    // Advance the frame counter
    m_frameIndex = (m_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

/**
 * @brief Reads the bytes of a specified file.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/01_Shader_modules.html
 */
std::vector<char> Renderer::ReadFile(const std::string &filePath) {
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
[[nodiscard]] vk::raii::ShaderModule Renderer::CreateShaderModule(
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

/**
 * @brief Create the graphics pipeline for rendering.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/00_Introduction.html
 */
void Renderer::CreateGraphicsPipeline() {
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
 * @brief Create a Vulkan command pool.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void Renderer::CreateCommandPool() {
    uint32_t queueIndex = m_vulkanContext.GetQueueIndex();
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queueIndex
    };

    m_commandPool = vk::raii::CommandPool(device, poolInfo);
}

/**
 * @brief Allocate a Vulkan command buffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void Renderer::CreateCommandBuffer() {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = m_commandPool,
        /**
         * The level determines if the command buffer is a primary or secondary command buffer.
         * 
         * - vk::CommandBufferLevel::ePrimary:
         *      Can be submitted to a command queue.
         * 
         * - vk::CommandBufferLevel::eSecondary:
         *      Cannot be submitted to a command queue but can be called from primary command
         *      buffers.
         */
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT
    };

    m_commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
}

/**
 * @brief Record a Vulkan command buffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void Renderer::RecordCommandBuffer(uint32_t imageIndex) {
    std::vector<vk::Image> swapChainImages = m_swapChain.GetImages();
    vk::Extent2D swapChainExtent = m_swapChain.GetExtent();
    const std::vector<vk::raii::ImageView>& swapChainImageViews = m_swapChain.GetImageViews();

    auto &commandBuffer = m_commandBuffers[m_frameIndex];
    commandBuffer.begin({});
    
    // Swapchain image: undefined -> vk::ImageLayout::eColorAttachmentOptimal
    TransitionImageLayout(
        imageIndex,
        vk::ImageLayout::eUndefined, // Old layout
        vk::ImageLayout::eColorAttachmentOptimal, // New layout
        {}, // srcAccessMask
        vk::AccessFlagBits2::eColorAttachmentWrite, // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput // dstStage
    );

    vk::ClearValue clearColor = vk::ClearColorValue(
        /* Black color */
        0.0f, 0.0f, 0.0f, 1.0f
    );

    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = swapChainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        // Image preprocessing
        .loadOp = vk::AttachmentLoadOp::eClear,
        // Image postprocessing
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::RenderingInfo renderingInfo = {
        .renderArea = {
            .offset = {0, 0},
            .extent = swapChainExtent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    commandBuffer.beginRendering(renderingInfo);
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        *m_graphicsPipeline
    );
    commandBuffer.setViewport(
        0,
        vk::Viewport(
            0.0f,
            0.0f,
            static_cast<float>(swapChainExtent.width),
            static_cast<float>(swapChainExtent.height),
            0.0f,
            1.0f
        )
    );
    commandBuffer.setScissor(
        0,
        vk::Rect2D(
            vk::Offset2D(0, 0),
            swapChainExtent
        )
    );

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRendering();

    // Swapchain image: vk::ImageLayout::eColorAttachmentOptimal -> vk::ImageLayout::ePresentSrcKHR
    TransitionImageLayout (
        imageIndex,
        vk::ImageLayout::eColorAttachmentOptimal, // Old layout
        vk::ImageLayout::ePresentSrcKHR, // New layout
        vk::AccessFlagBits2::eColorAttachmentWrite, // srcAccessMask
        {}, // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe // dstStage
    );

    commandBuffer.end();
}

/**
 * @brief Transition a Vulkan image layout to and from being suitable for rendering.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void Renderer::TransitionImageLayout(
    uint32_t imageIndex,
    vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout,
    vk::AccessFlags2 srcAccessMask,
    vk::AccessFlags2 dstAccessMask,
    vk::PipelineStageFlags2 srcStageMask,
    vk::PipelineStageFlags2 dstStageMask
) {
    std::vector<vk::Image> swapChainImages = m_swapChain.GetImages();

	vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapChainImages[imageIndex],
        .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1}};
    
    vk::DependencyInfo dependency_info = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier};
    
    m_commandBuffers[m_frameIndex].pipelineBarrier2(dependency_info);
}

/**
 * @brief Create Vulkan synchronization objects.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html
 */
void Renderer::CreateSyncObjects() {
    std::vector<vk::Image> swapChainImages = m_swapChain.GetImages();
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        m_renderFinishedSemaphores.emplace_back(
            device,
            vk::SemaphoreCreateInfo()
        );
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_presentCompleteSemaphores.emplace_back(
            device,
            vk::SemaphoreCreateInfo()
        );
        m_inFlightFences.emplace_back(
            device,
            vk::FenceCreateInfo{
                .flags = vk::FenceCreateFlagBits::eSignaled
            }
        );
    }
}
