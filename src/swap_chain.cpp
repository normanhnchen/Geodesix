#include "swap_chain.hpp"


SwapChain::SwapChain(Window& window, VulkanContext& vulkanContext)
    : m_window(window), m_vulkanContext(vulkanContext) {
}

void SwapChain::Init() {
    const vk::raii::PhysicalDevice& physicalDevice = m_vulkanContext.GetPhysicalDevice();
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    const vk::raii::SurfaceKHR& surface = m_vulkanContext.GetSurface();

    CreateSwapChain(physicalDevice, device, surface);
    CreateImageViews(device);
}


/**
 * @brief Creates the Vulkan swap chain.
 * 
 * The Vulkan swap chain is a queue (most of the time) that swaps rendered images in its queue to
 * the window surface (see SwapChain::CreateSurface). This way, only complete images are
 * displayed and rendering can occur before the image refreshes to prevent screen tearing (in the
 * commonly-used swap chain modes).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html
 */
void SwapChain::CreateSwapChain(
    const vk::raii::PhysicalDevice& physicalDevice,
    const vk::raii::Device& device,
    const vk::raii::SurfaceKHR& surface
) {
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    m_swapChainExtent = ChooseSwapExtent(surfaceCapabilities);
    uint32_t minImageCount = ChooseSwapMinImageCount(surfaceCapabilities);
    
    std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
    m_swapChainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

    std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
    vk::PresentModeKHR presentMode = ChooseSwapPresentMode(availablePresentModes);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{
        .surface = *surface,
        .minImageCount = minImageCount,
        .imageFormat = m_swapChainSurfaceFormat.format,
        .imageColorSpace = m_swapChainSurfaceFormat.colorSpace,
        .imageExtent = m_swapChainExtent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = true,
        /**
         * !!!!!!!!!!!!!!!!!!!!
         * NOTE: ignore for now
         * !!!!!!!!!!!!!!!!!!!!
         */
        .oldSwapchain = nullptr
    };

    m_swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    m_swapChainImages = m_swapChain.getImages();
}

/**
 * @brief Chooses a window surface (see SwapChain::CreateSurface) format for the swap chain,
 * preferred by if color formatting is more accurate.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html
 */
vk::SurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(
    std::vector<vk::SurfaceFormatKHR> const &availableFormats
) {
    // Make sure there is an available format
    assert(!availableFormats.empty());

    // Checked if the preferred SRGB format is available
    const auto formatIt = std::ranges::find_if(
        availableFormats,
        [](const auto &format) {
            return (
                /* SRGB results in more accurate perceived colors */
                format.format == vk::Format::eB8G8R8A8Srgb &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear
            );
        }
    );

    if (formatIt != availableFormats.end()) {
        // Return the preferred available format
        return *formatIt;
    } else {
        // By default settle with the first available format
        return availableFormats[0];
    }
}

/**
 * @brief Chooses a present mode for the swap chain.
 * 
 * The present mode for the window surface is used when displaying an image from the Vulkan queue (
 * see SwapChain::CreateLogicalDevice).
 * 
 * The function chooses vk::PresentModeKHR::eMailbox (triple buffering) by default if available.
 * Otherwise, it uses vk::PresentModeKHR::eFifo (double buffering) which is guaranteed to be
 * available.
 * 
 * The following present modes can be used:
 * 
 * - vk::PresentModeKHR::eImmediate
 *      Swap chain images are displayed immediately which may result in screen tearing.
 * 
 * - vk::PresentModeKHR::eFifo
 *      The swap chain is a queue of images where the screen displays images refreshed from the
 *      queue. When the queue is full, it blocks the application. This mode is similar to
 *      vertical sync.
 * 
 * - vk::PresentModeKHR::eFifoRelaxed
 *      This mode is the same as the vk::PresentModeKHR::eFifo except when waiting for the
 *      queue to fill, the image is displayed immediately which may result in screen tearing.
 * 
 * - vk::PresentModeKHR::eMailbox
 *      This is another variation of the vk::PresentModeKHR::eFifo except when the queue is
 *      full images already in the queue are replaced with newer ones. This mode is commonly
 *      known as triple buffering. This mode can be slightly more demanding to use than the
 *      FIFO mode.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html
 */
vk::PresentModeKHR SwapChain::ChooseSwapPresentMode(
    std::vector<vk::PresentModeKHR> const &availablePresentModes
) {
#ifdef DEBUG_PRESENT_IMMEDIATE
    return vk::PresentModeKHR::eImmediate;
#endif
#ifdef DEBUG_PRESENT_FIFO
    return vk::PresentModeKHR::eFifo;
#endif
#ifdef DEBUG_PRESENT_FIFO_RELAXED
    return vk::PresentModeKHR::eFifoRelaxed;
#endif
#ifdef DEBUG_PRESENT_MAILBOX
    return vk::PresentModeKHR::eMailbox;
#endif

    // Make sure there is an available present mode
    assert(std::ranges::any_of(
        availablePresentModes,
        [](auto presentMode) {
            return presentMode == vk::PresentModeKHR::eFifo;
        }
    ));
    if (std::ranges::any_of(
        availablePresentModes,
        [](const vk::PresentModeKHR value) {
            return vk::PresentModeKHR::eMailbox == value;
        }
    )) {
        return vk::PresentModeKHR::eMailbox;
    } else {
        // Default to vk::PresentModeKHR::eFifo; guaranteed to be available
        return vk::PresentModeKHR::eFifo;
    }
}

/**
 * @brief Chooses a Vulkan window extent (screen size in pixels) to draw to the window surface (see
 * SwapChain::CreateSurface).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html
 */
vk::Extent2D SwapChain::ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities) {
    // The window's extent is only std::numeric_limits<uint32_t>::max() if the
    // surface does not already want a fixed size
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        /* The surface already wants an exact size */
        return capabilities.currentExtent;
    } else {
        /* The surface has no size it specifically wants */

        int width, height;
        m_window.GetFramebufferSize(&width, &height);

        // Clamp to the surface's support range
        return {
            std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
    }
}

/**
 * @brief Chooses a specific minimum number of images to use in the swap chain (see
 * SwapChain::CreateSwapChain).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html
 */
uint32_t SwapChain::ChooseSwapMinImageCount(
    vk::SurfaceCapabilitiesKHR const &surfaceCapabilities
) {
    /**
     * The actual minimum number may cause the driver to wait before getting another image,
     * therefore the Vulkan tutorial recommends to request one more than the minimum. Also, the
     * minimum number of images must be less than the max amount supported by the surface.
     */

#ifdef DEBUG_MIN_IMAGE_COUNT_LEGACY
    /* Tutorial's explanatory version */

    uint32_t minImageCount = surfaceCapabilities.minImageCount + 1u;
#else
    /* Tutorial's actual shipped version */
    
    uint32_t minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
#endif

    // NOTE: Vulkan's maxImageCount == 0 means that there is no upper limit
    if (
        (0 < surfaceCapabilities.maxImageCount) &&
        (minImageCount > surfaceCapabilities.maxImageCount)
    ) {
        minImageCount = surfaceCapabilities.maxImageCount;
    }

    return minImageCount;
}

/**
 * Create Vulkan ImageViews to fill up the swap chain.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/02_Image_views.html
 */
void SwapChain::CreateImageViews(const vk::raii::Device& device) {
    // Make sure there is atleast an ImageView
    assert(m_swapChainImageViews.empty());

    vk::ImageViewCreateInfo imageViewCreateInfo{
        // 2D screen
        .viewType = vk::ImageViewType::e2D,
        .format = m_swapChainSurfaceFormat.format,
        .subresourceRange = {
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
        }
    };

    // The color channels can be swizzled around here
    imageViewCreateInfo.components = {
        /* Use the default mapping: vk::ComponentSwizzle::eIdentity */
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity
    };

    // The image's details and access can be described here
    imageViewCreateInfo.subresourceRange = {
        // Color target
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        // Use no mipmapping levels
        .levelCount = 1,
        // Use no multiple layers; only one layer
        .layerCount = 1
    };

    // Add the ImageViews to the swap chain
    for (auto &image : m_swapChainImages) {
        imageViewCreateInfo.image = image;
        // Add inplace to the end of the ImageViews to add to the swap chain
        m_swapChainImageViews.emplace_back(
            device,
            imageViewCreateInfo
        );
    }
}
