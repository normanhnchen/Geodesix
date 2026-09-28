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
    m_commandContext.Init();
    m_bufferContext.RetrieveCommandContext(m_commandContext);
    m_bufferContext.Init();
    m_window.RetrieveBufferContext(m_bufferContext);
    m_pipeline.Init();
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
    const vk::raii::Semaphore& semaphore = m_syncContext.GetSemaphore();
    const std::vector<vk::raii::Fence>& inFlightFences = m_syncContext.GetInFlightFences();
    const std::vector<vk::raii::CommandBuffer>& commandBuffers = m_commandContext.GetCommandBuffers();
    const std::vector<vk::raii::CommandBuffer>& computeCommandBuffers = m_commandContext.GetComputeCommandBuffers();
    uint64_t& timelineValue = m_syncContext.GetTimelineValue();

    m_syncContext.WaitForFences(m_frameIndex);
    m_syncContext.ResetFences(m_frameIndex);

    auto imageIndexOpt = m_syncContext.AcquireNextImageIndex(m_frameIndex);

    if (imageIndexOpt == std::nullopt || m_bufferContext.FramebufferResized()) {
        /* The swap chain is incompatible with the surface and can no longer be used to render */
        m_bufferContext.RecreateComputeStorageImage();
        // Start a fresh frame
        return;
    }

    uint32_t imageIndex = *imageIndexOpt;

    // Wait for the image acquire before touching the image
    m_syncContext.WaitForFences(m_frameIndex);

    m_bufferContext.UpdateCameraUbo();
    m_bufferContext.UpdateParameterUbo(0.1);
    m_bufferContext.MapComputeUboMemory(m_frameIndex);

    /* Update timeline semaphore values for this frame */
    uint64_t computeWaitValue = timelineValue;
    uint64_t computeSignalValue = ++timelineValue;
    uint64_t graphicsWaitValue = computeSignalValue;
    uint64_t graphicsSignalValue = ++timelineValue;

    {
        /* Compute Command Buffers */

        m_commandContext.RecordComputeCommandBuffer(imageIndex, m_frameIndex);

        vk::TimelineSemaphoreSubmitInfo computeTimelineInfo{
            .waitSemaphoreValueCount = 1,
            .pWaitSemaphoreValues = &computeWaitValue,
            .signalSemaphoreValueCount = 1,
            .pSignalSemaphoreValues = &computeSignalValue
        };

        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eComputeShader};

        vk::SubmitInfo computeSubmitInfo{
            .pNext = &computeTimelineInfo,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &*semaphore,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &*computeCommandBuffers[m_frameIndex],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &*semaphore
        };

        queue.submit(computeSubmitInfo, nullptr);
    }
    
    {
        /* ---- Graphics Command Buffers ---- */

        m_commandContext.RecordCommandBuffer(imageIndex, m_frameIndex);
    
        vk::TimelineSemaphoreSubmitInfo graphicsTimelineInfo{
            .waitSemaphoreValueCount = 1,
            .pWaitSemaphoreValues = &graphicsWaitValue,
            .signalSemaphoreValueCount = 1,
            .pSignalSemaphoreValues = &graphicsSignalValue
        };

        vk::PipelineStageFlags waitDestinationStageMask(
            vk::PipelineStageFlagBits ::eColorAttachmentOutput
        );
        const vk::SubmitInfo graphicsSubmitInfo{
            .pNext = &graphicsTimelineInfo,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &*semaphore,
            .pWaitDstStageMask = &waitDestinationStageMask,
            .commandBufferCount = 1,
            .pCommandBuffers = &*commandBuffers[m_frameIndex],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &*semaphore
        };

        queue.submit(graphicsSubmitInfo, nullptr);
    
        /* Wait for the graphics rendering to finish */
        vk::SemaphoreWaitInfo waitInfo{
            .semaphoreCount = 1,
            .pSemaphores = &*semaphore,
            .pValues = &graphicsSignalValue
        };
        auto result = device.waitSemaphores(waitInfo, UINT64_MAX);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to wait for the Vulkan semaphore!");
        }


        const vk::PresentInfoKHR presentInfoKHR{
            /* No binary semaphore is needed; we are using a timeline semaphore*/
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .swapchainCount = 1,
            .pSwapchains = &*swapChain,
            .pImageIndices = &imageIndex
        };

        result = queue.presentKHR(presentInfoKHR);

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
    }

    // Advance the frame counter
    m_frameIndex = (m_frameIndex + 1) % m_syncContext.maxFramesInFlight;
}
