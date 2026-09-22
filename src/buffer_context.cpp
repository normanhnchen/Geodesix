#include <vector>

#include "buffer_context.hpp"
#include "vertex.hpp"


BufferContext::BufferContext(VulkanContext& vulkanContext)
    : m_vulkanContext(vulkanContext) {
}

void BufferContext::Init() {
    CreateVertexBuffer(vertex_data::vertices);
}

const vk::raii::Buffer& BufferContext::GetVertexBuffer() const {
    return m_vertexBuffer;
}

/**
 * @brief Creates a Vulkan vertex buffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/01_Vertex_buffer_creation.html
 */
void BufferContext::CreateVertexBuffer(std::vector<Vertex> vertices) {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

	vk::BufferCreateInfo bufferInfo{
        // Size of the buffer in bytes
        .size = sizeof(vertices[0]) * vertices.size(),
        .usage = vk::BufferUsageFlagBits::eVertexBuffer,
        .sharingMode = vk::SharingMode::eExclusive
    };

    m_vertexBuffer = vk::raii::Buffer(device, bufferInfo);

    vk::MemoryRequirements memRequirements = m_vertexBuffer.getMemoryRequirements();

    vk::MemoryAllocateInfo memoryAllocateInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = FindMemoryType(
            memRequirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
        )
    };

    vertexBufferMemory = vk::raii::DeviceMemory(device, memoryAllocateInfo);
    m_vertexBuffer.bindMemory(
        *vertexBufferMemory,
        0 // We are allocating specifically for this vertex buffer; no memory offset
    );

    void* data = vertexBufferMemory.mapMemory(0, bufferInfo.size);
    memcpy(data, vertices.data(), bufferInfo.size);
    vertexBufferMemory.unmapMemory();
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
