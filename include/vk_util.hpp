#pragma once


#include <string>

#include "header_inclusions/vulkan.hpp"

#include "swap_chain.hpp"
#include "command_context.hpp"


namespace vk_util {


std::vector<char> ReadFile(const std::string &filePath);

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
);

uint32_t FindMemoryType(
    uint32_t typeFilter,
    vk::MemoryPropertyFlags properties,
    const vk::raii::PhysicalDevice& physicalDevice
);


} // namespace vk_util
