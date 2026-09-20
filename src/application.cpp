#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <limits>
#include <map>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application.hpp"


/**
 * @brief Runs the application.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
void Application::Run() {
    m_window.Init();
    InitVulkan();
    MainLoop();
    Cleanup();
}

/**
 * @brief Initializes the Vulkan library and calls all of the required helper initialization
 * functions in the *required dependency order*:
 * 
 * (Instance -> (Validation Layers & Debug messenger) -> Surface -> Physical Device -> Logical
 *  Device -> Swap Chain -> Image Views)
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
void Application::InitVulkan() {
    m_vulkanContext.Init();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateCommandBuffer();
    CreateSyncObjects();
}

/**
 * @brief Main rendering loop.
 * 
 * TODO: currently only polls GLFW events; add actual rendering implementations and Vulkan API
 * calls.
 */
void Application::MainLoop() {
    while (!m_window.ShouldClose()) {
        m_window.PollEvents();
        DrawFrame();
    }

    // Wait for the logical device to finish its operations before terminating
    m_vulkanContext.WaitForDevice();
}

/**
 * @brief Destroys the window and terminates GLFW before terminating the program. Vulkan resources
 * are cleaned up via RAII.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
void Application::Cleanup() {
    CleanupSwapChain();

    m_window.Cleanup();
}

/**
 * @brief Create a Vulkan shader module from code (in bytes).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/01_Shader_modules.html
 */
[[nodiscard]] vk::raii::ShaderModule Application::CreateShaderModule(const std::vector<char>& code) const {
    vk::ShaderModuleCreateInfo createInfo{
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t *>(code.data())
    };
    vk::raii::ShaderModule shaderModule{m_device, createInfo};

    return shaderModule;
}

/**
 * @brief Create the graphics pipeline for rendering.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/00_Introduction.html
 */
void Application::CreateGraphicsPipeline() {
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

    m_pipelineLayout = vk::raii::PipelineLayout(m_device, pipelineLayoutInfo);

    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &m_swapChainSurfaceFormat.format
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
        m_device,
        nullptr,
        pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>()
    );
}

/**
 * @brief Create a Vulkan command pool.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void Application::CreateCommandPool() {
    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_queueIndex
    };

    m_commandPool = vk::raii::CommandPool(m_device, poolInfo);
}

/**
 * @brief Allocate a Vulkan command buffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void Application::CreateCommandBuffer() {
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

    m_commandBuffers = vk::raii::CommandBuffers(m_device, allocInfo);
}

/**
 * @brief Record a Vulkan command buffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void Application::RecordCommandBuffer(uint32_t imageIndex) {
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
        .imageView = m_swapChainImageViews[imageIndex],
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
            .extent = m_swapChainExtent
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
            static_cast<float>(m_swapChainExtent.width),
            static_cast<float>(m_swapChainExtent.height),
            0.0f,
            1.0f
        )
    );
    commandBuffer.setScissor(
        0,
        vk::Rect2D(
            vk::Offset2D(0, 0),
            m_swapChainExtent
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
void Application::TransitionImageLayout(
    uint32_t imageIndex,
    vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout,
    vk::AccessFlags2 srcAccessMask,
    vk::AccessFlags2 dstAccessMask,
    vk::PipelineStageFlags2 srcStageMask,
    vk::PipelineStageFlags2 dstStageMask
) {
	vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_swapChainImages[imageIndex],
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
void Application::DrawFrame() {
    /*  Wait until the previous frame is finished */
    auto fenceResult = m_device.waitForFences(
        *m_inFlightFences[m_frameIndex],
        vk::True,
        UINT64_MAX // Timeout
    );
    if (fenceResult != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence!");
    }

    auto [result, imageIndex] = m_swapChain.acquireNextImage(
        UINT64_MAX, // Timeout
        *m_presentCompleteSemaphores[m_frameIndex],
        nullptr
    );

    if (result == vk::Result::eErrorOutOfDateKHR) {
        /* The swap chain is incompatible with the surface and can no longer be used to render. */
        RecreateSwapChain();
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        /* The surface properties don't match anymore. */
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("Failed to get a new swap chain image!");
    }

    // Prevent a deadlock by resetting the fence only when we know it will be submitting work later
    // (we acquired the next image and it passed all of the error checks)
    m_device.resetFences(*m_inFlightFences[m_frameIndex]);

    RecordCommandBuffer(imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput
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

    m_queue.submit(submitInfo, *m_inFlightFences[m_frameIndex]);

    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_renderFinishedSemaphores[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &*m_swapChain,
        .pImageIndices = &imageIndex
    };

    result = m_queue.presentKHR(presentInfoKHR);

    if (
        // The swap chain is incompatible with the surface and can no longer be used to render
        (result == vk::Result::eSuboptimalKHR) ||
        // The surface properties don't match anymore
        (result == vk::Result::eErrorOutOfDateKHR) ||
        m_window.Resized()
    ) {
        m_window.ResetResizedFlag();
        RecreateSwapChain();
    } else {
        // On any other error besides eSuccess, presentKHR throws an exception
        assert(result == vk::Result::eSuccess);
    }

    // Advance the frame counter
    m_frameIndex = (m_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

/**
 * @brief Create Vulkan synchronization objects.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html
 */
void Application::CreateSyncObjects() {
    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        m_renderFinishedSemaphores.emplace_back(
            m_device,
            vk::SemaphoreCreateInfo()
        );
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_presentCompleteSemaphores.emplace_back(
            m_device,
            vk::SemaphoreCreateInfo()
        );
        m_inFlightFences.emplace_back(
            m_device,
            vk::FenceCreateInfo{
                .flags = vk::FenceCreateFlagBits::eSignaled
            }
        );
    }
}

/**
 * @brief Recreate the Vulkan swap chain and its corresponding objects.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/04_Swap_chain_recreation.html
 */
void Application::RecreateSwapChain() {
    // Pause until the window is unminimized
    m_window.MinimizedLoop();
    if (m_window.ShouldClose()) {
        return;
    }

    m_device.waitIdle();

    CleanupSwapChain();

    CreateSwapChain();
    CreateImageViews();
}

/**
 * @brief Clean up swap chain objects.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/04_Swap_chain_recreation.html
 */
void Application::CleanupSwapChain() {
    m_swapChainImageViews.clear();
    m_swapChain = nullptr;
}
