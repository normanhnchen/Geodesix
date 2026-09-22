#include <vector>

#include "buffer_context.hpp"
#include "buffer_data.hpp"
#include "command_context.hpp"


class CommandContext; // Forward declaration

BufferContext::BufferContext(VulkanContext& vulkanContext)
    : m_vulkanContext(vulkanContext) {
}

void BufferContext::Init() {
    CreateVertexBuffer(buffer_data::vertex::vertices);
    CreateIndexBuffer(buffer_data::index::indices);
}

/**
 * @brief Copies the CommandContext object internally.
 * 
 * Needed as the BufferContext and CommandContext objects circularly depend on each other.
 */
void BufferContext::RetrieveCommandContext(CommandContext& commandContext) {
    m_commandContext = &commandContext;
}

const vk::raii::Buffer& BufferContext::GetVertexBuffer() const {
    return m_vertexBuffer;
}

const vk::raii::Buffer& BufferContext::GetIndexBuffer() const {
    return m_indexBuffer;
}

/**
 * @brief Creates an arbitrary Vulkan buffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/02_Staging_buffer.html
 */
std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> BufferContext::CreateBuffer(
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties
) {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    vk::BufferCreateInfo bufferInfo{
        // Size of the buffer in bytes
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };
    vk::raii::Buffer buffer = vk::raii::Buffer(device, bufferInfo);
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties)
    };
    vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
    buffer.bindMemory(
        *bufferMemory,
        // We are allocating specifically for this vertex buffer; no memory offset
        0
    );
    return {std::move(buffer), std::move(bufferMemory)};
}

void BufferContext::CopyBuffer(
    vk::raii::Buffer& srcBuffer,
    vk::raii::Buffer& dstBuffer,
    vk::DeviceSize size
) {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    const vk::raii::Queue& queue = m_vulkanContext.GetQueue();
    const vk::raii::CommandPool& commandPool = m_commandContext->GetCommandPool();

    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    vk::raii::CommandBuffer commandCopyBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());

    commandCopyBuffer.begin({
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    });

    commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));

    commandCopyBuffer.end();

    // Execute the command buffer
    queue.submit(vk::SubmitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandCopyBuffer
    }, nullptr);
    // Wait until the command buffer has finished executing
    queue.waitIdle();
}

/**
 * @brief Creates a Vulkan vertex buffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/02_Staging_buffer.html
 */
void BufferContext::CreateVertexBuffer(std::vector<Vertex> vertices) {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    auto [stagingBuffer, stagingBufferMemory] = CreateBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent
    );

    void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(dataStaging, vertices.data(), bufferSize);
    stagingBufferMemory.unmapMemory();

    std::tie(m_vertexBuffer, m_vertexBufferMemory) = CreateBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    // Move the vertex data into the device local buffer
    CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);
}

void BufferContext::CreateIndexBuffer(std::vector<uint16_t> indices) {
		vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		auto [stagingBuffer, stagingBufferMemory] = CreateBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
        );

		void *data = stagingBufferMemory.mapMemory(0, bufferSize);
		memcpy(data, indices.data(), (size_t) bufferSize);
		stagingBufferMemory.unmapMemory();

		std::tie(m_indexBuffer, m_indexBufferMemory) = CreateBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eIndexBuffer |
            vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );

		CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);
}

/**
 * @brief Find the right type of memory to use (that is compatible with the GPU).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/01_Vertex_buffer_creation.html
 */
uint32_t BufferContext::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    const vk::raii::PhysicalDevice& physicalDevice = m_vulkanContext.GetPhysicalDevice();

    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    /* Find a memory type suitable for the buffer */
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (
            // The memory type is suitable if the bit is 1
            (typeFilter & (1 << i)) &&
            // The memory type is suitable if it matches our requested properties
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties
        ) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}
