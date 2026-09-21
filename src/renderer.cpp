#include "renderer.hpp"


Renderer::Renderer(
    Window& window,
    VulkanContext& vulkanContext,
    SwapChain& swapChain,
    Pipeline& pipeline,
    SyncContext& syncContext
)
    : m_window(window),
    m_vulkanContext(vulkanContext),
    m_swapChain(swapChain),
    m_pipeline(pipeline),
    m_syncContext(syncContext) {
}

void Renderer::Init() {
    m_pipeline.Init();
    CreateCommandPool();
    CreateCommandBuffer();
    m_syncContext.Init();
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
    const std::vector<vk::raii::Semaphore>& presentCompleteSemaphores = m_syncContext.GetPresentCompleteSemaphore();
    const std::vector<vk::raii::Semaphore>& renderFinishedSemaphores = m_syncContext.GetRenderFinishedSemaphores();
    const std::vector<vk::raii::Fence>& inFlightFences = m_syncContext.GetInFlightFences();

    m_syncContext.WaitForFences(m_frameIndex);

    auto imageIndexOpt = m_syncContext.AcquireNextImageIndex(m_frameIndex);

    if (imageIndexOpt == std::nullopt) {
        /* The swap chain is incompatible with the surface and can no longer be used to render */
        return;
    }

    uint32_t imageIndex = *imageIndexOpt;

    m_syncContext.ResetFences(m_frameIndex);

    RecordCommandBuffer(imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask(
        vk::PipelineStageFlagBits ::eColorAttachmentOutput
    );
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*presentCompleteSemaphores[m_frameIndex],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*m_commandBuffers[m_frameIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*renderFinishedSemaphores[imageIndex]
    };

    queue.submit(submitInfo, *inFlightFences[m_frameIndex]);

    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphores[imageIndex],
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
    const vk::raii::Pipeline& graphicsPipeline = m_pipeline.GetGraphicsPipeline();

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
        *graphicsPipeline
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

    commandBuffer.draw(4, 1, 0, 0);

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
