#pragma once


#include "vulkan_context.hpp"
#include "swap_chain.hpp"


class SyncContext {
public:
    SyncContext(VulkanContext& vulkanContext, SwapChain& swapChain);

    void Init();

    void CreateObjects();

    void WaitForFences(uint32_t frameIndex);
    void ResetFences(uint32_t frameIndex);

    std::optional<uint32_t> AcquireNextImageIndex(uint32_t frameIndex);

    const std::vector<vk::raii::Semaphore>& GetPresentCompleteSemaphore() const;
    const std::vector<vk::raii::Semaphore>& GetRenderFinishedSemaphores() const;
    const std::vector<vk::raii::Fence>& GetInFlightFences() const;

    uint32_t maxFramesInFlight = 2;

private:
    VulkanContext& m_vulkanContext;
    SwapChain& m_swapChain;

    std::vector<vk::raii::Semaphore> m_presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_inFlightFences;
};
