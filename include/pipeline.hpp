#pragma once


#include <iostream>
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "vulkan_context.hpp"
#include "swap_chain.hpp"


class Pipeline {
public:
    Pipeline(VulkanContext& vulkanContext, SwapChain& swapChain);

    void Init();

    const vk::raii::Pipeline& GetGraphicsPipeline() const;

private:
    VulkanContext& m_vulkanContext;
    SwapChain& m_swapChain;

    vk::raii::PipelineLayout m_pipelineLayout = nullptr;
    vk::raii::Pipeline m_graphicsPipeline = nullptr;

    void Create();

    [[nodiscard]] vk::raii::ShaderModule CreateShaderModule(
        const std::vector<char>& code
    ) const;
};
