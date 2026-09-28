#pragma once


#include <filesystem>

#include "header_inclusions/vulkan.hpp"

#include "vulkan_context.hpp"
#include "buffer_data.hpp"
#include "sync_context.hpp"
#include "swap_chain.hpp"
#include "window.hpp"


const std::filesystem::path ASSETS_DIR = CMAKE_ASSETS_DIR;
const std::string ENV_PATH = std::string(ASSETS_DIR / "starmap_4k.exr");

// Forward declaration
// Used because BufferContext and CommandContext circularly depend on eachother
class CommandContext;

class BufferContext {
public:
    BufferContext(
        VulkanContext& vulkanContext,
        SyncContext& syncContext,
        SwapChain& swapChain,
        Window& window
    );

    void Init();

    void UpdateUniformBuffer(uint32_t frameIndex);
    void UpdateComputeUniformBuffer(uint32_t frameIndex);

    void RetrieveCommandContext(CommandContext& commandContext);

    const vk::raii::Buffer& GetVertexBuffer() const;
    const vk::raii::Buffer& GetIndexBuffer() const;

    const vk::raii::DescriptorSetLayout& GetDescriptorSetLayout() const;
    const std::vector<vk::raii::DescriptorSet>& GetDescriptorSets() const;

    const vk::raii::DescriptorSetLayout& GetComputeDescriptorSetLayout() const;
    const std::vector<vk::raii::DescriptorSet>& GetComputeDescriptorSets() const;

    const vk::raii::Image& GetComputeStorageImage() const;
    const vk::raii::Sampler& GetComputeStorageImageSampler() const;

    vk::Extent2D GetComputeStorageImageExtent();

    bool m_computeImageInitialized = false;

private:
    VulkanContext& m_vulkanContext;
    CommandContext* m_commandContext = nullptr;
    SyncContext& m_syncContext;
    SwapChain& m_swapChain;
    Window& m_window;

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

    std::vector<vk::raii::Buffer> m_shaderStorageBuffers;
    std::vector<vk::raii::DeviceMemory> m_shaderStorageBuffersMemory;

    std::vector<vk::raii::Buffer> m_computeUniformBuffers;
    std::vector<vk::raii::DeviceMemory> m_computeUniformBuffersMemory;
    std::vector<void*> m_computeUniformBuffersMapped;

    vk::raii::DescriptorSetLayout m_computeDescriptorSetLayout = nullptr;
    std::vector<vk::raii::DescriptorSet> m_computeDescriptorSets;

    vk::raii::Image m_computeStorageImage = nullptr;
    vk::raii::DeviceMemory m_computeStorageImageMemory = nullptr;
    vk::raii::ImageView m_computeStorageImageView = nullptr;
    vk::raii::Sampler m_computeStorageImageSampler = nullptr;

    vk::raii::Image m_envImage = nullptr;
    vk::raii::DeviceMemory m_envImageMemory = nullptr;
    vk::raii::ImageView m_envImageView = nullptr;
    vk::raii::Sampler m_envImageSampler = nullptr;

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

    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateUniformBuffers();

    void CreateDescriptorSetLayout();
    void CreateDescriptorPool();
    void CreateDescriptorSets();

    void CreateComputeUniformBuffers();

    void CreateComputeDescriptorSetLayout();
    void CreateComputeDescriptorSets();

    void CreateComputeStorageImage();
    void CreateEnvironmentTexture(const std::string& filePath);
};
