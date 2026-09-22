#include "command_context.hpp"
#include "vk_util.hpp"
#include "buffer_data.hpp"
#include "buffer_context.hpp"


CommandContext::CommandContext(
    VulkanContext& vulkanContext,
    SwapChain& swapChain,
    Pipeline& pipeline,
    SyncContext& syncContext,
    BufferContext& BufferContext
)
    : m_vulkanContext(vulkanContext),
    m_swapChain(swapChain),
    m_pipeline(pipeline),
    m_syncContext(syncContext),
    m_bufferContext(BufferContext) {
}

void CommandContext::Init() {
    CreateCommandPool();
    CreateCommandBuffer();
}

/**
 * @brief Record a Vulkan command buffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void CommandContext::RecordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex) {
    std::vector<vk::Image> swapChainImages = m_swapChain.GetImages();
    vk::Extent2D swapChainExtent = m_swapChain.GetExtent();
    const std::vector<vk::raii::ImageView>& swapChainImageViews = m_swapChain.GetImageViews();
    const vk::raii::Pipeline& graphicsPipeline = m_pipeline.GetGraphicsPipeline();
    const vk::raii::Buffer& vertexBuffer = m_bufferContext.GetVertexBuffer();
    const vk::raii::Buffer& indexBuffer = m_bufferContext.GetIndexBuffer();

    auto &commandBuffer = m_commandBuffers[frameIndex];
    commandBuffer.begin({});
    
    // Swapchain image: undefined -> vk::ImageLayout::eColorAttachmentOptimal
    vk_util::TransitionImageLayout(
        imageIndex,
        frameIndex,
        vk::ImageLayout::eUndefined, // Old layout
        vk::ImageLayout::eColorAttachmentOptimal, // New layout
        {}, // srcAccessMask
        vk::AccessFlagBits2::eColorAttachmentWrite, // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // dstStage
        m_swapChain,
        *this
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

    commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
    commandBuffer.bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint16);
    
    commandBuffer.drawIndexed(
        static_cast<uint32_t>(buffer_data::index::indices.size()),
        1, 0, 0, 0
    );

    commandBuffer.endRendering();

    // Swapchain image: vk::ImageLayout::eColorAttachmentOptimal -> vk::ImageLayout::ePresentSrcKHR
    vk_util::TransitionImageLayout (
        imageIndex,
        frameIndex,
        vk::ImageLayout::eColorAttachmentOptimal, // Old layout
        vk::ImageLayout::ePresentSrcKHR, // New layout
        vk::AccessFlagBits2::eColorAttachmentWrite, // srcAccessMask
        {}, // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe, // dstStage
        m_swapChain,
        *this
    );

    commandBuffer.end();
}

const std::vector<vk::raii::CommandBuffer>& CommandContext::GetCommandBuffers() const {
    return m_commandBuffers;
}

const vk::raii::CommandPool& CommandContext::GetCommandPool() const {
    return m_commandPool;
}

/**
 * @brief Create a Vulkan command pool.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void CommandContext::CreateCommandPool() {
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
void CommandContext::CreateCommandBuffer() {
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
        .commandBufferCount = m_syncContext.maxFramesInFlight
    };

    m_commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
}
