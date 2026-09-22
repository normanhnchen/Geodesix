#pragma once


#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "vulkan_context.hpp"
#include "vertex.hpp"


class CommandContext; // Forward declaration

class BufferContext {
public:
    BufferContext(VulkanContext& vulkanContext);

    void Init();

    void RetrieveCommandContext(CommandContext& commandContext);

    const vk::raii::Buffer& GetVertexBuffer() const;

private:
    VulkanContext& m_vulkanContext;
    CommandContext* m_commandContext = nullptr;

    vk::raii::Buffer m_vertexBuffer = nullptr;
    vk::raii::DeviceMemory vertexBufferMemory = nullptr;

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
    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
};
