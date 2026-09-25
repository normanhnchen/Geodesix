#pragma once


#include "header_inclusions/vulkan.hpp"

#include "vulkan_context.hpp"
#include "buffer_data.hpp"
#include "sync_context.hpp"
#include "swap_chain.hpp"


class CommandContext; // Forward declaration

class BufferContext {
public:
    BufferContext(
        VulkanContext& vulkanContext,
        SyncContext& syncContext,
        SwapChain& swapChain
    );

    void Init();

    void UpdateUniformBuffer(uint32_t frameIndex);

    void RetrieveCommandContext(CommandContext& commandContext);

    const vk::raii::Buffer& GetVertexBuffer() const;
    const vk::raii::Buffer& GetIndexBuffer() const;

    const vk::raii::DescriptorSetLayout& GetDescriptorSetLayout() const;
    const std::vector<vk::raii::DescriptorSet>& GetDescriptorSets() const;

private:
    VulkanContext& m_vulkanContext;
    CommandContext* m_commandContext = nullptr;
    SyncContext& m_syncContext;
    SwapChain& m_swapChain;

    vk::raii::Buffer m_vertexBuffer = nullptr;
    vk::raii::DeviceMemory m_vertexBufferMemory = nullptr;

    vk::raii::Buffer m_indexBuffer = nullptr;
    vk::raii::DeviceMemory m_indexBufferMemory = nullptr;

    std::vector<vk::raii::Buffer> m_uniformBuffers;
    std::vector<vk::raii::DeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;

    vk::raii::DescriptorSetLayout m_descriptorSetLayout = nullptr;
    vk::raii::DescriptorPool m_descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSet> m_descriptorSets;

    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties
    );

    void CopyBuffer(
        vk::raii::Buffer& srcBuffer,
        vk::raii::Buffer& dstBuffer,
        vk::DeviceSize size
    );

    void CreateVertexBuffer(std::vector<Vertex> vertices);
    void CreateIndexBuffer(std::vector<uint16_t> indices);
    void CreateUniformBuffers(auto ubo);

    void CreateDescriptorSetLayout();
    void CreateDescriptorPool();
    void CreateDescriptorSets(auto ubo);
};
