#pragma once


#include <iostream>
#include <vector>

#include "header_inclusions/vulkan.hpp"

#include "vulkan_context.hpp"
#include "swap_chain.hpp"
#include "buffer_context.hpp"


class Pipeline {
public:
    Pipeline(
        VulkanContext& vulkanContext,
        SwapChain& swapChain,
        BufferContext& bufferContext
    );

    void Init();

    const vk::raii::Pipeline& GetGraphicsPipeline() const;
    const vk::raii::PipelineLayout& GetLayout() const;

private:
    VulkanContext& m_vulkanContext;
    SwapChain& m_swapChain;
    BufferContext& m_bufferContext;

    vk::raii::PipelineLayout m_pipelineLayout = nullptr;
    vk::raii::Pipeline m_graphicsPipeline = nullptr;

    void Create();

    [[nodiscard]] vk::raii::ShaderModule CreateShaderModule(
        const std::vector<char>& code
    ) const;
};
