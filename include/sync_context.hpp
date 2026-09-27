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

    const vk::raii::Semaphore& GetSemaphore() const;

    std::optional<uint32_t> AcquireNextImageIndex(uint32_t frameIndex);

    const std::vector<vk::raii::Fence>& GetInFlightFences() const;

    uint64_t& GetTimelineValue();

    uint32_t maxFramesInFlight = 2;

private:
    VulkanContext& m_vulkanContext;
    SwapChain& m_swapChain;

    vk::raii::Semaphore m_semaphore = nullptr;
	uint64_t m_timelineValue = 0;

    std::vector<vk::raii::Fence> m_inFlightFences;
};
