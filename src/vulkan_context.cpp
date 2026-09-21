#include <iostream>

#include "vulkan_context.hpp"


VulkanContext::VulkanContext(Window& window)
    : m_window(window) {
}

void VulkanContext::Init() {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    SelectPhysicalDevice();
    CreateLogicalDevice();
}

/**
 * @brief Waits for the logical device to finish its operations.
 */
void VulkanContext::WaitForDevice() {
    m_device.waitIdle();
}

const vk::raii::PhysicalDevice& VulkanContext::GetPhysicalDevice() const {
    return m_physicalDevice;
}

const vk::raii::Device& VulkanContext::GetDevice() const {
    return m_device;
}

const vk::raii::SurfaceKHR& VulkanContext::GetSurface() const {
    return m_surface;
}

const vk::raii::Queue& VulkanContext::GetQueue() const {
    return m_queue;
}

uint32_t VulkanContext::GetQueueIndex() {
    return m_queueIndex;
}

/**
 * @brief Initializes the Vulkan instance.
 * 
 * The Vulkan instance bridges the gap between the Vulkan API and the application.
 * 
 * The function validates that the required layers and extensions are supported before instance
 * creation, then creates the instance with a Vulkan RAII object for automatic destruction.
 * 
 * @throws std::runtime_error if a required validation layer or extension is unsupported.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/01_Instance.html
 */
void VulkanContext::CreateInstance() {
    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName = "Geodesix",
        .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .apiVersion = vk::ApiVersion14
    };

    // Get the required layers
    std::vector<char const*> requiredLayers;

#ifdef DEBUG_VALIDATION_LAYERS
    requiredLayers.assign(validationLayers.begin(), validationLayers.end());
#endif

    // Check if the required layers are supported by the Vulkan implementation.
    auto layerProperties = m_context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(
        requiredLayers,
        [&layerProperties](auto const &requiredLayer) {
            return std::ranges::none_of(
                layerProperties,
                [requiredLayer](auto const &layerProperty) {
                    // Validate the layer
                    return strcmp(layerProperty.layerName, requiredLayer) == 0;
                }
            );
        }
    );
    if (unsupportedLayerIt != requiredLayers.end()) {
        throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
    }

    std::vector<const char*> requiredExtensions = GetRequiredInstanceExtensions();

    // Check if the required extensions are supported by the Vulkan implementation
    auto extensions = m_context.enumerateInstanceExtensionProperties();
    
#ifdef DEBUG_PRINT_EXTENSIONS
    // Debug: print available extensions to the console
    std::cout << "Available extensions:\n";
    for (const auto& extension : extensions) {
        std::cout << "\t" << extension.extensionName << "\n";
    }
#endif
    
    // Verify all required extensions
    for (const char* requiredExtension : requiredExtensions) {
        if (std::ranges::none_of(
            extensions,
            [requiredExtension](auto const& extensionProperty) {
                // Validate the extension
                return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
            })
        ) {
            throw std::runtime_error(std::string("Required GLFW extension not supported: ") + requiredExtension);
        }
    }

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };

#ifdef __APPLE__
    // Add macOS / MoltenVK portability extension bit to prevent the possible error:
    // vk::Result::eErrorIncompatibleDriver
    createInfo.flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

    m_instance = vk::raii::Instance(m_context, createInfo);
}

/**
 * @brief Initializes the debug messenger for the validation layers.
 * 
 * Because the Vulkan API is designed to have minimal driver overhead and be as performant as
 * possible, errors can be silent and will result in crashes or unexpected behavior. Therefore,
 * validation layers (they are optional) are used to check for errors.
 * 
 * The debug messenger returns error messages to the terminal that are caught by the validation
 * layers.
 * 
 * The severity of the message is specified with one of the following flags:
 * 
 * - vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
 *      Diagnostic message from Vulkan components (e.g. loader, layers, drivers)
 * 
 * - vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
 *      Informational message (e.g. creation of a resource)
 * 
 * - vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
 *      Message about behavior that may come from an application bug
 * 
 * - vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
 *      Message about behavior that is invalid
 * 
 * The message type is specified with one of the following values:
 * 
 * - vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral:
 *      Some event has happened that is unrelated to the specification or performance
 * 
 * - vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation:
 *      Something has happened that violates the specification or indicates a possible mistake
 * 
 * - vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance:
 *      Potential non-optimal use of Vulkan
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/02_Validation_layers.html
 */
void VulkanContext::SetupDebugMessenger() {
#ifndef DEBUG_VALIDATION_LAYERS
    return;
#endif

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
    );
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
    );
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
        .messageSeverity = severityFlags,
        .messageType = messageTypeFlags,
        .pfnUserCallback = &DebugCallback
    };
    m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(
        debugUtilsMessengerCreateInfoEXT
    );
}

/**
 * @brief Gets the required Vulkan and GLFW extensions before Vulkan instance creation.
 * 
 * This function is used to get the required extensions during instance creation (see
 * VulkanContext::CreateInstance).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/01_Instance.html
 */
std::vector<const char*> VulkanContext::GetRequiredInstanceExtensions() {
    // Get the required instance extensions from GLFW
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef __APPLE__
    // Add macOS / MoltenVK portability extension to prevent the possible error:
    // vk::Result::eErrorIncompatibleDriver
    requiredExtensions.push_back(vk::KHRPortabilityEnumerationExtensionName);
#endif

#ifdef DEBUG_VALIDATION_LAYERS
    // Add debug messenger (callback) for the validation layers
    requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
#endif

    return requiredExtensions;
}

/**
 * @brief Debug messenger callback for the validation layers.
 * 
 * See VulkanContext::SetupDebugMessenger for the debug messenger creation.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/02_Validation_layers.html
 */
VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanContext::DebugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    if (
        severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning ||
        severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
    ) {
        // Message is important enough to show because of the severity
        std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
    }

    return vk::False;
}

/**
 * @brief Selects the best Vulkan physical device to use.
 * 
 * The Vulkan physical device object is only used for retrieving its properties and capabilities to
 * be used for operations. The Vulkan logical device (see VulkanContext::CreateLogicalDevice) uses
 * the physical device's features and will be used as the handle for Vulkan operations after the
 * physical device has been initiated.
 * 
 * The function selects the device candidate based on its "score," influenced by the device's
 * properties and if it is a discrete GPU.
 * 
 * @throws std::runtime_error if no suitable GPU was found.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/03_Physical_devices_and_queue_families.html
 */
void VulkanContext::SelectPhysicalDevice() {
    auto physicalDevices = m_instance.enumeratePhysicalDevices();
    if (physicalDevices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, vk::raii::PhysicalDevice> candidates;
    for (const auto& pd : physicalDevices) {
        if (!IsDeviceSuitable(pd)) {
            continue;
        }
        auto deviceProperties = pd.getProperties();
        auto deviceFeatures = pd.getFeatures();
        uint32_t score = 0;

        // Discrete GPUs are significantly more performant
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        }

        // Maximum possible size of textures
        score += deviceProperties.limits.maxImageDimension2D;

        candidates.insert(std::make_pair(score, pd));
    }

    // Check if the best candidate is suitable
    if (!candidates.empty()) {
        // Get the key of the last entry in the list (the best candidate)
        if (candidates.rbegin()->first > 0) {
            // Get the best candidate's value
            m_physicalDevice = candidates.rbegin()->second;
        }
    } else {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

/**
 * @brief Checks if a Vulkan physical device object has the required Vulkan extensions and features
 * needed for the application.
 * 
 * This function is used during Vulkan physical device selection (see
 * VulkanContext::SelectPhysicalDevice).
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/03_Physical_devices_and_queue_families.html
 */
bool VulkanContext::IsDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice) {
    // Check if the device supports Vulkan API version 1.3
    bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    bool supportsGraphics = std::ranges::any_of(
        queueFamilies, [](auto const &qfp) {
            return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
        }
    );

    // Check if each required device extension is supported by the physical device
    auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions = std::ranges::all_of(
        requiredDeviceExtension,
        [&availableDeviceExtensions](
            auto const & requiredDeviceExtension
        ) {
            return std::ranges::any_of(
                availableDeviceExtensions,
                [requiredDeviceExtension](
                    auto const & availableDeviceExtension
                ) {
                    return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0;
                }
            );
        }
    );

    // Check if the required features are supported by the physical device
    auto features = physicalDevice.template
        getFeatures2<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        >();
    bool supportsRequiredFeatures = (
        features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
        features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
        features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState
    );

    // Check if the physical device meets all of the required criteria
    if (
        supportsVulkan1_3 &&
        supportsGraphics &&
        supportsAllRequiredExtensions &&
        supportsRequiredFeatures
    ) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Creates the Vulkan logical device and specifies the queue families used.
 * 
 * The Vulkan logical device uses the physical device's features and is used as the handle for
 * Vulkan operations.
 * 
 * The Vulkan queue is an asynchronous execution queue that receives commands. Queues are decided
 * from queue families which represents a set of queues that support a specific set of operations.
 * 
 * @throws std::runtime_error if a Vulkan queue could not be found.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/04_Logical_device_and_queues.html
 */
void VulkanContext::CreateLogicalDevice() {
    // Get the first index of the queue with graphics capabilities
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

    // Bitwise operator; get the max possible uint32_t
    m_queueIndex = ~0;

    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
        if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
            m_physicalDevice.getSurfaceSupportKHR(qfpIndex, *m_surface)) {
            // Found a queue family that supports both Vulkan graphics and present surfaces
            m_queueIndex = qfpIndex;
            break;
        }
    }
    if (m_queueIndex == ~0) {
        throw std::runtime_error("Could not find a queue for Vulkan graphics and present surfaces!");
    }

    // Decide which queues have relative priority over other queues
    float queuePriorities[] = {
        0.5f
    };

    vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
        .queueFamilyIndex = m_queueIndex,
        .queueCount = 1,
        .pQueuePriorities = queuePriorities
    };

    // Create a chain of feature structures
    vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
    >
    featureChain = {
        /* Physical device features */
        {
#ifdef POLYGON_FILL
            // Empty
#else
            // Anything other than vk::PolygonMode::eFill must enable the fillModeNonSolid feature
            .features = {
                .fillModeNonSolid = true
            }
#endif
        },
        /* Vulkan 1.1 features */
        {
            .shaderDrawParameters = true
        },
        /* Vulkan 1.3 feature */
        {
            .synchronization2 = true,
            .dynamicRendering = true
        },
        /* Extended dynamic state features */
        {
            .extendedDynamicState = true
        }
    };

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
        .ppEnabledExtensionNames = requiredDeviceExtension.data()
    };

    m_device = vk::raii::Device(m_physicalDevice, deviceCreateInfo);
    m_queue = vk::raii::Queue(m_device, m_queueIndex, 0);
}

/**
 * @brief Creates the Vulkan RAII window surface object.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/00_Window_surface.html
 */
void VulkanContext::CreateSurface() {
    VkSurfaceKHR surface = m_window.CreateVulkanSurface(*m_instance);
    m_surface = vk::raii::SurfaceKHR(m_instance, surface);
}
