#include <vector>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "buffer_context.hpp"
#include "buffer_data.hpp"
#include "command_context.hpp"
#include "vk_util.hpp"


class CommandContext; // Forward declaration

BufferContext::BufferContext(
    VulkanContext& vulkanContext,
    SyncContext& syncContext,
    SwapChain& swapChain
)
    : m_vulkanContext(vulkanContext),
    m_syncContext(syncContext),
    m_swapChain(swapChain) {
}

void BufferContext::Init() {
    CreateVertexBuffer(buffer_data::vertex::vertices);
    CreateIndexBuffer(buffer_data::index::indices);
    CreateUniformBuffers(buffer_data::uniform::UniformBufferObject{});
    CreateDescriptorSetLayout();
    CreateDescriptorPool();
    CreateDescriptorSets(buffer_data::uniform::UniformBufferObject{});
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/00_Descriptor_set_layout_and_buffer.html
 */
void BufferContext::UpdateUniformBuffer(uint32_t frameIndex) {
    vk::Extent2D swapChainExtent = m_swapChain.GetExtent();

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<
            float, std::chrono::seconds::period
        >(currentTime - startTime).count();

    buffer_data::uniform::UniformBufferObject ubo{};
    ubo.model = rotate(
        glm::mat4(1.0f),
        time * glm::radians(90.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    ubo.view = lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    ubo.proj = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height),
        0.1f,
        10.0f
    );

    memcpy(m_uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
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

const vk::raii::DescriptorSetLayout& BufferContext::GetDescriptorSetLayout() const {
    return m_descriptorSetLayout;
}

const std::vector<vk::raii::DescriptorSet>& BufferContext::GetDescriptorSets() const {
    return m_descriptorSets;
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
    const vk::raii::PhysicalDevice& physicalDevice = m_vulkanContext.GetPhysicalDevice();

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
        .memoryTypeIndex = vk_util::FindMemoryType(
            memRequirements.memoryTypeBits,
            properties,
            physicalDevice
        )
    };
    vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
    buffer.bindMemory(
        *bufferMemory,
        // We are allocating specifically for this vertex buffer; no memory offset
        0
    );
    return {std::move(buffer), std::move(bufferMemory)};
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/02_Staging_buffer.html
 */
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

/**
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/03_Index_buffer.html
 */
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
 * @see https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/00_Descriptor_set_layout_and_buffer.html
 */
void BufferContext::CreateUniformBuffers(auto ubo) {
    for (size_t i = 0; i < m_syncContext.maxFramesInFlight; i++) {
        vk::DeviceSize bufferSize = sizeof(ubo);
        auto [buffer, bufferMem] = CreateBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
        );
        m_uniformBuffers.emplace_back(std::move(buffer));
        m_uniformBuffersMemory.emplace_back(std::move(bufferMem));
        m_uniformBuffersMapped.emplace_back(
            m_uniformBuffersMemory.back().mapMemory(0, bufferSize)
        );
    }
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/00_Descriptor_set_layout_and_buffer.html
 */
void BufferContext::CreateDescriptorSetLayout() {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    vk::DescriptorSetLayoutBinding uboLayoutBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex
    };
    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding
    };
    m_descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/01_Descriptor_pool_and_sets.html
 */
void BufferContext::CreateDescriptorPool() {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    vk::DescriptorPoolSize poolSize{
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = m_syncContext.maxFramesInFlight
    };

    vk::DescriptorPoolCreateInfo poolInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = m_syncContext.maxFramesInFlight,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };

    m_descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/01_Descriptor_pool_and_sets.html
 */
void BufferContext::CreateDescriptorSets(auto ubo) {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    std::vector<vk::DescriptorSetLayout> layouts(
        m_syncContext.maxFramesInFlight,
        *m_descriptorSetLayout
    );
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };

    m_descriptorSets = device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < m_syncContext.maxFramesInFlight; i++) {
        vk::DescriptorBufferInfo bufferInfo{
            .buffer = m_uniformBuffers[i],
            .offset = 0,
            .range = sizeof(ubo)
        };
        vk::WriteDescriptorSet descriptorWrite{
            .dstSet = m_descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &bufferInfo
        };
        device.updateDescriptorSets(descriptorWrite, {});
    }
}
