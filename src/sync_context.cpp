#include <iostream>
#include <stdexcept>
#include <optional>

#include "header_inclusions/vulkan.hpp"

#include "sync_context.hpp"


SyncContext::SyncContext(VulkanContext& vulkanContext, SwapChain& swapChain)
    : m_vulkanContext(vulkanContext), m_swapChain(swapChain) {
}

void SyncContext::Init() {
    CreateObjects();
}

/**
 * @brief Create Vulkan synchronization objects.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html
 */
void SyncContext::CreateObjects() {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    
    vk::SemaphoreTypeCreateInfo semaphoreType{ 
        .semaphoreType = vk::SemaphoreType::eTimeline,
        .initialValue = 0
    };
    m_semaphore = vk::raii::Semaphore(device, {.pNext = &semaphoreType});
    m_timelineValue = 0;

    for (size_t i = 0; i < maxFramesInFlight; i++) {
        m_inFlightFences.emplace_back(
            device,
            vk::FenceCreateInfo{
                .flags = vk::FenceCreateFlagBits::eSignaled
            }
        );
    }
}

/**
 * @brief Wait until the previous frame is finished.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html
 */
void SyncContext::WaitForFences(uint32_t frameIndex) {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    auto fenceResult = device.waitForFences(
        *m_inFlightFences[frameIndex],
        vk::True,
        UINT64_MAX // Timeout
    );
    if (fenceResult != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence!");
    }
}

/**
 * @brief Prevent a deadlock by resetting the fence only when we know it will be submitting work later
 * (we acquired the next image and it passed all of the error checks).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html
 */
void SyncContext::ResetFences(uint32_t frameIndex) {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    device.resetFences(*m_inFlightFences[frameIndex]);
}

const vk::raii::Semaphore& SyncContext::GetSemaphore() const {
    return m_semaphore;
}

std::optional<uint32_t> SyncContext::AcquireNextImageIndex(uint32_t frameIndex) {
    auto imageIndexOpt = m_swapChain.AcquireNextImageIndex(
        m_inFlightFences[frameIndex]
    );

    if (imageIndexOpt == std::nullopt) {
        /* The swap chain is incompatible with the surface and can no longer be used to render */
        return std::nullopt;
    }

    uint32_t imageIndex = *imageIndexOpt;

    return imageIndex;
}

const std::vector<vk::raii::Fence>& SyncContext::GetInFlightFences() const {
    return m_inFlightFences;
}

/**
 * @brief Return the timeline value's reference because it will be mutated in the rendering draw
 * loop.
 */
uint64_t& SyncContext::GetTimelineValue() {
    return m_timelineValue;
}
