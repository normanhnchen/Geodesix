#include <iostream>
#include <stdexcept>
#include <optional>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

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
    std::vector<vk::Image> swapChainImages = m_swapChain.GetImages();
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        m_renderFinishedSemaphores.emplace_back(
            device,
            vk::SemaphoreCreateInfo()
        );
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_presentCompleteSemaphores.emplace_back(
            device,
            vk::SemaphoreCreateInfo()
        );
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

std::optional<uint32_t> SyncContext::AcquireNextImageIndex(uint32_t frameIndex) {
    auto imageIndexOpt = m_swapChain.AcquireNextImageIndex(
        m_presentCompleteSemaphores[frameIndex]
    );

    if (imageIndexOpt == std::nullopt) {
        /* The swap chain is incompatible with the surface and can no longer be used to render */
        return std::nullopt;
    }

    uint32_t imageIndex = *imageIndexOpt;

    return imageIndex;
}

const std::vector<vk::raii::Semaphore>& SyncContext::GetPresentCompleteSemaphore() const {
    return m_presentCompleteSemaphores;
}

const std::vector<vk::raii::Semaphore>& SyncContext::GetRenderFinishedSemaphores() const {
    return m_renderFinishedSemaphores;
}

const std::vector<vk::raii::Fence>& SyncContext::GetInFlightFences() const {
    return m_inFlightFences;
}
