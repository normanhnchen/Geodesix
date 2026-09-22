#include "renderer.hpp"


Renderer::Renderer(
    Window& window,
    VulkanContext& vulkanContext,
    SwapChain& swapChain,
    Pipeline& pipeline,
    CommandContext& commandContext,
    SyncContext& syncContext,
    BufferContext& bufferContext
)
    : m_window(window),
    m_vulkanContext(vulkanContext),
    m_swapChain(swapChain),
    m_pipeline(pipeline),
    m_commandContext(commandContext),
    m_syncContext(syncContext),
    m_bufferContext(bufferContext) {
}

void Renderer::Init() {
    m_pipeline.Init();
    m_bufferContext.Init();
    m_commandContext.Init();
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
    const std::vector<vk::raii::CommandBuffer>& commandBuffers = m_commandContext.GetCommandBuffers();

    m_syncContext.WaitForFences(m_frameIndex);

    auto imageIndexOpt = m_syncContext.AcquireNextImageIndex(m_frameIndex);

    if (imageIndexOpt == std::nullopt) {
        /* The swap chain is incompatible with the surface and can no longer be used to render */
        return;
    }

    uint32_t imageIndex = *imageIndexOpt;

    m_syncContext.ResetFences(m_frameIndex);

    m_commandContext.RecordCommandBuffer(imageIndex, m_frameIndex);

    vk::PipelineStageFlags waitDestinationStageMask(
        vk::PipelineStageFlagBits ::eColorAttachmentOutput
    );
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*presentCompleteSemaphores[m_frameIndex],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffers[m_frameIndex],
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
    m_frameIndex = (m_frameIndex + 1) % m_syncContext.maxFramesInFlight;
}
