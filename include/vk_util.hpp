#pragma once


#include <string>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

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


} // namespace vk_util
