#include <vector>
#include <filesystem>
#include <fstream>

#include "vk_util.hpp"


namespace vk_util {


/**
 * @brief Reads the bytes of a specified file.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/01_Shader_modules.html
 */
std::vector<char> ReadFile(const std::string &filePath) {
    // Open the file at the end (ate) in binary mode
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file!");
    }
    // Get the exact number of bytes the file has and allocate it to a buffer
    std::vector<char> buffer(file.tellg());
    // Reset the read cursor to beginning
    file.seekg(0, std::ios::beg);
    // Read the raw bytes
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();
    return buffer;
}

/**
 * @brief Transition a Vulkan image layout to and from being suitable for rendering.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 */
void TransitionImageLayout(
    uint32_t imageIndex,
    uint32_t frameIndex,
    vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout,
    vk::AccessFlags2 srcAccessMask,
    vk::AccessFlags2 dstAccessMask,
    vk::PipelineStageFlags2 srcStageMask,
    vk::PipelineStageFlags2 dstStageMask,
    SwapChain& swapChain,
    CommandContext& commandContext
) {
    std::vector<vk::Image> swapChainImages = swapChain.GetImages();
    const std::vector<vk::raii::CommandBuffer>& commandBuffers = commandContext.GetCommandBuffers();

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
    
    commandBuffers[frameIndex].pipelineBarrier2(dependency_info);
}

/**
 * @brief Find the right type of memory to use (that is compatible with the GPU).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/01_Vertex_buffer_creation.html
 */
uint32_t FindMemoryType(
    uint32_t typeFilter,
    vk::MemoryPropertyFlags properties,
    const vk::raii::PhysicalDevice& physicalDevice
) {
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


} // namespace vk_util
