#pragma once


#include <vector>
#include <iostream>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "window.hpp"


/* ==== Debug Macros ==== */

#define DEBUG_VALIDATION_LAYERS
// #define DEBUG_PRINT_EXTENSIONS

/**
 * ---- Present Modes ----
 * These macros are used in Application::ChooseSwapPresentMode
 */
// #define DEBUG_PRESENT_IMMEDIATE
// #define DEBUG_PRESENT_FIFO
// #define DEBUG_PRESENT_FIFO_RELAXED
// #define DEBUG_PRESENT_MAILBOX

/**
 * The macro below defines the minimum image count strategy used in
 * Application::ChooseSwapMinImageCount. If the macro is defined,  we use the tutorial's
 * explanatory minimum count minImageCount + 1. Otherwise, we use the tutorial's shipped version
 * std::max(3u, surfaceCapabilities.minImageCount).
 */
// #define DEBUG_MIN_IMAGE_COUNT_LEGACY

/**
 * ---- Topology Modes ---
 * These macros are used in Appication::CreateGraphicsPipeline for the inputAssembly struct.
 * 
 * They represent how the geometry will be drawn from vertices sent to the GPU and the primitive.
 * 
 * NOTE: only one of the macros should be defined or else it might lead to unexpected behavior!
 * 
 * NOTE: for any mode other than eFill, a physical device feature must be enabled (see
 * Application::CreateLogicalDevice -> featureChain struct)
 */

/** 
 * Draw points from vertices.
 * 
 * vk::PrimitiveTopology::ePointList
 */
// #define TOPOLOGY_POINT_LIST
/** 
 * Draw lines between every two vertices (without reusing vertices).
 * 
 * vk::PrimitiveTopology::eLineList
 */
// #define TOPOLOGY_LINE_LIST
/** 
 * Draw lines where the end vertex of every line is used as the start vertex for the next line.
 * 
 * vk::PrimitiveTopology::eLineStrip
 */
// #define TOPOLOGY_LINE_STRIP
/** 
 * Draw triangles from every three vertices (without reusing vertices).
 * 
 * vk::PrimitiveTopology::eTriangleList
 */
// #define TOPOLOGY_TRIANGLE_LIST
/** 
 * Draw triangles where every second and third vertex of every triangle are reused for the next
 * triangle's first two vertices.
 * 
 * vk::PrimitiveTopology::eTriangleStrip
 */
#define TOPOLOGY_TRIANGLE_STRIP

/**
 * ---- Polygon Mode ----
 * 
 * These macros are used in Application::CreateGraphicsPipeline for the rasterizer struct.
 * 
 * NOTE: only one of the macros should be defined or else it might lead to unexpected behavior!
 */

/**
 * Fill the area of polygons.
 * 
 * vk::PolygonMode::eFill
 */
#define POLYGON_FILL
/**
 * Draw polygon edges as lines.
 * 
 * vk::PolygonMode::eLine
 */
// #define POLYGON_LINE
/**
 * Draw polygon vertices as points.
 * 
 * vk::PolygonMode::ePoint
 */
// #define POLYGON_POINT

constexpr uint32_t WIDTH  = 800;
constexpr uint32_t HEIGHT = 600;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<char const*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::filesystem::path SHADER_SPIRV_DIR = CMAKE_SHADER_SPIRV_DIR;
const std::string SHADER_MAIN_VERT_PATH = std::string(SHADER_SPIRV_DIR / "main.vert.spv");
const std::string SHADER_MAIN_FRAG_PATH = std::string(SHADER_SPIRV_DIR / "main.frag.spv");

/**
 * The entry point is the function where the shader starts executing. In GLSL, there can only be
 * one entry point per file (that being void main()). In other languages like Slang, there can be
 * multiple entry points where each could represent a shader stage. For GLSL, we default to "main"
 * for all shader modules.
 */
constexpr const char* SHADER_ENTRY_POINT = "main";

/**
 * The main application, including a Vulkan & GLFW backend.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
class Application {
public:
    void Run();

private:
    Window m_window {WIDTH, HEIGHT, "Geodesix"};
    vk::raii::Context m_context;
	vk::raii::Instance m_instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger = nullptr;
    vk::raii::PhysicalDevice m_physicalDevice = nullptr;
    vk::raii::Device m_device = nullptr;
    vk::raii::Queue m_queue = nullptr;
    vk::raii::SurfaceKHR m_surface = nullptr;
    vk::raii::SwapchainKHR m_swapChain = nullptr;
    std::vector<vk::Image> m_swapChainImages;
    vk::SurfaceFormatKHR m_swapChainSurfaceFormat;
    vk::Extent2D m_swapChainExtent;
    std::vector<vk::raii::ImageView> m_swapChainImageViews;
    vk::raii::PipelineLayout m_pipelineLayout = nullptr;
    vk::raii::Pipeline m_graphicsPipeline = nullptr;
    vk::raii::CommandPool m_commandPool = nullptr;
    std::vector<vk::raii::CommandBuffer> m_commandBuffers;
    std::vector<vk::raii::Semaphore> m_presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_inFlightFences;

    uint32_t m_queueIndex = 0;
    uint32_t m_frameIndex = 0;

    std::vector<const char*> requiredDeviceExtension = {
        vk::KHRSwapchainExtensionName
    };

    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void Cleanup();

    void CreateInstance();
    void SetupDebugMessenger();
    std::vector<const char*> GetRequiredInstanceExtensions();

    /**
     * @brief Debug messenger callback for the validation layers.
     * 
     * See Application::SetupDebugMessenger for the debug messenger creation.
     * 
     * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/02_Validation_layers.html
     */
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
        void * pUserData
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

    void SelectPhysicalDevice();
    bool IsDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);

    void CreateLogicalDevice();

    void CreateSurface();

    void CreateSwapChain();
    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
    vk::PresentModeKHR ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
    vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
    uint32_t ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

    void CreateImageViews();

    /**
     * @brief Reads the bytes of a specified file.
     * 
     * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/01_Shader_modules.html
     */
    static std::vector<char> ReadFile(const std::string &filePath) {
        // Open the file at the end (ate) in binary mode
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file!");
		}
        // Get the exact number of bytes the file has and allocate it to a buffer
		std::vector<char> buffer(file.tellg());
        // Reset the read cursor to beginning
		file.seekg(0, std::ios::beg);
        // Read the raw bytes
		file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
		file.close();
		return buffer;
	}

    [[nodiscard]] vk::raii::ShaderModule CreateShaderModule(const std::vector<char>& code) const;

    void CreateGraphicsPipeline();

    void CreateCommandPool();
    void CreateCommandBuffer();
    void RecordCommandBuffer(uint32_t imageIndex);
    void TransitionImageLayout(
        uint32_t imageIndex,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask
    );

    void DrawFrame();
    void CreateSyncObjects();

    void RecreateSwapChain();
    void CleanupSwapChain();
};
