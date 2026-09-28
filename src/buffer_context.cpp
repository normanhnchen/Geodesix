#include <vector>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "buffer_context.hpp"
#include "buffer_data.hpp"
#include "command_context.hpp"
#include "vk_util.hpp"
#include "exr_loading.hpp"


// Forward declaration
// Used because BufferContext and CommandContext circularly depend on eachother
class CommandContext;

BufferContext::BufferContext(
    VulkanContext& vulkanContext,
    SyncContext& syncContext,
    SwapChain& swapChain,
    Window& window,
    Camera& camera
)
    : m_vulkanContext(vulkanContext),
    m_syncContext(syncContext),
    m_swapChain(swapChain),
    m_window(window),
    m_camera(camera) {
}

void BufferContext::Init() {
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateEnvironmentTexture(ENV_PATH);
    CreateComputeStorageImage();
    CreateGraphicsDescriptorSetLayout();
    CreateDescriptorPool();
    CreateGraphicsDescriptorSets();
    CreateComputeUniformBuffers();
    CreateComputeDescriptorSetLayout();
    CreateComputeDescriptorSets();
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/00_Descriptor_set_layout_and_buffer.html
 */
void BufferContext::UpdateCameraUbo() {
    m_cameraUbo.pos = glm::vec3(0.0);
    m_cameraUbo.right = m_camera.m_right;
    m_cameraUbo.up = m_camera.m_up;
    m_cameraUbo.front = m_camera.m_front;
    m_cameraUbo.fov = glm::radians(45.0f);
}

void BufferContext::UpdateParameterUbo(float deltaTime) {
    m_parameterUbo.deltaTime = deltaTime;
    m_parameterUbo.time += m_parameterUbo.deltaTime;
}

void BufferContext::MapComputeUboMemory(uint32_t frameIndex) {
    memcpy(m_cameraUbosMapped[frameIndex], &m_cameraUbo, sizeof(m_cameraUbo));
    memcpy(m_parameterUbosMapped[frameIndex], &m_parameterUbo, sizeof(m_parameterUbo));
}

/**
 * @brief Copies the CommandContext object internally.
 * 
 * Needed as the BufferContext and CommandContext objects circularly depend on each other.
 */
void BufferContext::RetrieveCommandContext(CommandContext& commandContext) {
    m_commandContext = &commandContext;
}

const vk::raii::Buffer& BufferContext::GetVertexBuffer() const {
    return m_vertexBuffer;
}

const vk::raii::Buffer& BufferContext::GetIndexBuffer() const {
    return m_indexBuffer;
}

const vk::raii::DescriptorSetLayout& BufferContext::GetGraphicsDescriptorSetLayout() const {
    return m_graphicsDescriptorSetLayout;
}

const std::vector<vk::raii::DescriptorSet>& BufferContext::GetGraphicsDescriptorSets() const {
    return m_graphicsDescriptorSets;
}

const vk::raii::DescriptorSetLayout& BufferContext::GetComputeDescriptorSetLayout() const {
    return m_computeDescriptorSetLayout;
}

const std::vector<vk::raii::DescriptorSet>& BufferContext::GetComputeDescriptorSets() const {
    return m_computeDescriptorSets;
}

const vk::raii::Image& BufferContext::GetComputeStorageImage() const {
    return m_computeStorageImage;
}

const vk::raii::Sampler& BufferContext::GetComputeStorageImageSampler() const {
    return m_computeStorageImageSampler;
}

vk::Extent2D BufferContext::GetComputeStorageImageExtent() {
    int width; int height;
    m_window.GetFramebufferSize(&width, &height);

    return vk::Extent2D(
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    );
}

/**
 * @brief Creates an arbitrary Vulkan buffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/02_Staging_buffer.html
 */
std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> BufferContext::CreateBuffer(
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties
) {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    const vk::raii::PhysicalDevice& physicalDevice = m_vulkanContext.GetPhysicalDevice();

    vk::BufferCreateInfo bufferInfo{
        .size = size,
        .usage = usage,
        // Specify that access can only be exclusive to one queue family at a time
        .sharingMode = vk::SharingMode::eExclusive
    };
    vk::raii::Buffer buffer = vk::raii::Buffer(device, bufferInfo);
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vk_util::FindMemoryType(
            memRequirements.memoryTypeBits,
            properties,
            physicalDevice
        )
    };
    vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
    buffer.bindMemory(
        *bufferMemory,
        // We are allocating specifically for this vertex buffer; no memory offset
        0
    );
    return {std::move(buffer), std::move(bufferMemory)};
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/02_Staging_buffer.html
 */
void BufferContext::CopyBuffer(
    vk::raii::Buffer& srcBuffer,
    vk::raii::Buffer& dstBuffer,
    vk::DeviceSize size
) {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    const vk::raii::Queue& queue = m_vulkanContext.GetQueue();
    const vk::raii::CommandPool& commandPool = m_commandContext->GetCommandPool();

    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    vk::raii::CommandBuffer commandCopyBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());

    commandCopyBuffer.begin({
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    });

    commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));

    commandCopyBuffer.end();

    // Execute the command buffer
    queue.submit(vk::SubmitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandCopyBuffer
    }, nullptr);
    // Wait until the command buffer has finished executing
    queue.waitIdle();
}

/**
 * @brief Creates a Vulkan vertex buffer.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/02_Staging_buffer.html
 */
void BufferContext::CreateVertexBuffer() {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    std::vector<Vertex> vertices = buffer_data::vertex::vertices;

    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    auto [stagingBuffer, stagingBufferMemory] = CreateBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent
    );

    void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(dataStaging, vertices.data(), bufferSize);
    stagingBufferMemory.unmapMemory();

    std::tie(m_vertexBuffer, m_vertexBufferMemory) = CreateBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    // Move the vertex data into the device local buffer
    CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/03_Index_buffer.html
 */
void BufferContext::CreateIndexBuffer() {
    std::vector<uint16_t> indices = buffer_data::index::indices;

    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    auto [stagingBuffer, stagingBufferMemory] = CreateBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent
    );

    void *data = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(data, indices.data(), (size_t) bufferSize);
    stagingBufferMemory.unmapMemory();

    std::tie(m_indexBuffer, m_indexBufferMemory) = CreateBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/00_Descriptor_set_layout_and_buffer.html
 */
void BufferContext::CreateGraphicsDescriptorSetLayout() {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    std::array<vk::DescriptorSetLayoutBinding, 1> layoutBindings = {{
        {
            /* Combined image sampler */
            .binding = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eFragment
        }
    }};
    
    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
        .pBindings = layoutBindings.data()
    };

    m_graphicsDescriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/01_Descriptor_pool_and_sets.html
 */
void BufferContext::CreateDescriptorPool() {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    uint32_t maxFramesInFlight = m_syncContext.maxFramesInFlight;

    std::array poolSize{
        vk::DescriptorPoolSize(
            vk::DescriptorType::eUniformBuffer,
            maxFramesInFlight * 2
        ),
        vk::DescriptorPoolSize(
            vk::DescriptorType::eStorageImage,
            maxFramesInFlight
        ),
        vk::DescriptorPoolSize(
            vk::DescriptorType::eCombinedImageSampler,
            maxFramesInFlight
        )
    };

    vk::DescriptorPoolCreateInfo poolInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = m_syncContext.maxFramesInFlight * 2,
        .poolSizeCount = poolSize.size(),
        .pPoolSizes = poolSize.data()
    };

    m_descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/01_Descriptor_pool_and_sets.html
 */
void BufferContext::CreateGraphicsDescriptorSets() {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    std::vector<vk::DescriptorSetLayout> layouts(
        m_syncContext.maxFramesInFlight,
        *m_graphicsDescriptorSetLayout
    );
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };

    m_graphicsDescriptorSets = device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < m_syncContext.maxFramesInFlight; i++) {
        vk::DescriptorImageInfo samplerInfo{
            .sampler = *m_computeStorageImageSampler,
            .imageView = *m_computeStorageImageView,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };

        std::array descriptorWrites{
            vk::WriteDescriptorSet{
                .dstSet = m_graphicsDescriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &samplerInfo
            }
        };

        device.updateDescriptorSets(descriptorWrites, {});
    }
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/11_Compute_Shader.html
 */
void BufferContext::CreateComputeUniformBuffers() {
    for (size_t i = 0; i < m_syncContext.maxFramesInFlight; i++) {
        vk::DeviceSize cameraUboSize = sizeof(buffer_data::uniform::CameraUbo);
        auto [cameraUbo, cameraUboMemory] = CreateBuffer(
            cameraUboSize,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
        );
        m_cameraUbos.emplace_back(std::move(cameraUbo));
        m_cameraUbosMemory.emplace_back(std::move(cameraUboMemory));
        m_cameraUbosMapped.emplace_back(
            m_cameraUbosMemory.back().mapMemory(0, cameraUboSize)
        );

        vk::DeviceSize parameterUboSize = sizeof(buffer_data::uniform::CameraUbo);
        auto [parameterUbo, parameterUboMemory] = CreateBuffer(
            parameterUboSize,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
        );
        m_parameterUbos.emplace_back(std::move(parameterUbo));
        m_parameterUbosMemory.emplace_back(std::move(parameterUboMemory));
        m_parameterUbosMapped.emplace_back(
            m_parameterUbosMemory.back().mapMemory(0, parameterUboSize)
        );
    }
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/11_Compute_Shader.html
 */
void BufferContext::CreateComputeDescriptorSetLayout() {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    std::array<vk::DescriptorSetLayoutBinding, 4> layoutBindings {{
        {
            /* Camera UBO */
            .binding = 0,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute
        },
        {
            /* Parameter UBO */
            .binding = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute,
        },
        {
            /* Compute storage image */
            .binding = 2,
            .descriptorType = vk::DescriptorType::eStorageImage,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute, 
        },
        {
            /* Environment map */
            .binding = 3,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute,
        }
    }};

    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
        .pBindings = layoutBindings.data()
    };
    m_computeDescriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
}

/**
 * @see https://docs.vulkan.org/tutorial/latest/11_Compute_Shader.html
 */
void BufferContext::CreateComputeDescriptorSets() {
    uint32_t maxFramesInFlight = m_syncContext.maxFramesInFlight;
    const vk::raii::Device& device = m_vulkanContext.GetDevice();

    std::vector<vk::DescriptorSetLayout> layouts(maxFramesInFlight, m_computeDescriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = *m_descriptorPool;
    allocInfo.descriptorSetCount = maxFramesInFlight;
    allocInfo.pSetLayouts = layouts.data();
    m_computeDescriptorSets.clear();
    m_computeDescriptorSets = device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < maxFramesInFlight; i++) {
        // Camera UBO
        vk::DescriptorBufferInfo cameraUboInfo{
            .buffer = m_cameraUbos[i],
            .offset = 0,
            .range = sizeof(buffer_data::uniform::CameraUbo)
        };
        // Parameter UBO
        vk::DescriptorBufferInfo parameterUboInfo{
            m_parameterUbos[i],
            0,
            sizeof(buffer_data::uniform::ParameterUbo)
        };
        // Compute storage image
        vk::DescriptorImageInfo storageImageInfo{
            .imageView = *m_computeStorageImageView,
            .imageLayout = vk::ImageLayout::eGeneral
        };
        // Environment map
        vk::DescriptorImageInfo envInfo{
            .sampler = *m_envImageSampler,
            .imageView = *m_envImageView,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };

        std::array descriptorWrites{
            // Camera UBO
            vk::WriteDescriptorSet{
                .dstSet = m_computeDescriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo = &cameraUboInfo
            },
            // Parameter UBO
            vk::WriteDescriptorSet{
                .dstSet = *m_computeDescriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pImageInfo = nullptr,
                .pBufferInfo = &parameterUboInfo,
                .pTexelBufferView = nullptr
            },
            // Compute storage image
            vk::WriteDescriptorSet{
                .dstSet = *m_computeDescriptorSets[i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eStorageImage,
                .pImageInfo = &storageImageInfo,
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr
            },
            // Environment map
            vk::WriteDescriptorSet{
                .dstSet = *m_computeDescriptorSets[i],
                .dstBinding = 3,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &envInfo
            }
        };
        device.updateDescriptorSets(descriptorWrites, {});
    }
}

void BufferContext::CreateComputeStorageImage() {
    int width; int height;
    m_window.GetFramebufferSize(&width, &height);
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    const vk::raii::PhysicalDevice& physicalDevice = m_vulkanContext.GetPhysicalDevice();

    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        // High precision color format to prevent color compression and color banding
        .format = vk::Format::eR32G32B32A32Sfloat,
        .extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
            1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eSampled |
            vk::ImageUsageFlagBits::eStorage |
            vk::ImageUsageFlagBits::eTransferDst,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
    };

    m_computeStorageImage = vk::raii::Image(device, imageInfo);

    vk::MemoryRequirements memRequirements = m_computeStorageImage.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vk_util::FindMemoryType(
            memRequirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            physicalDevice
        )
    };
    m_computeStorageImageMemory = vk::raii::DeviceMemory(device, allocInfo);
    m_computeStorageImage.bindMemory(m_computeStorageImageMemory, 0);

    vk::ImageViewCreateInfo viewInfo{
        .image = *m_computeStorageImage,
        .viewType = vk::ImageViewType::e2D,
        // Must match the image's format exactly since the image object has an immutable format
        .format = vk::Format::eR32G32B32A32Sfloat,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    m_computeStorageImageView = vk::raii::ImageView(device, viewInfo);

    vk::SamplerCreateInfo samplerInfo{
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eClampToEdge,
        .addressModeV = vk::SamplerAddressMode::eClampToEdge,
        .addressModeW = vk::SamplerAddressMode::eClampToEdge,
        .anisotropyEnable = vk::False
    };

    m_computeStorageImageSampler = vk::raii::Sampler(device, samplerInfo);
}

void BufferContext::CreateEnvironmentTexture(const std::string& filePath) {
    const vk::raii::Device& device = m_vulkanContext.GetDevice();
    const vk::raii::PhysicalDevice& physicalDevice = m_vulkanContext.GetPhysicalDevice();
    const vk::raii::CommandPool& commandPool = m_commandContext->GetCommandPool();
    const vk::raii::Queue& queue = m_vulkanContext.GetQueue();

    ExrImage envExrImage = LoadExr(filePath);

    /* Staging buffer */
    auto [stagingBuffer, stagingMemory] = CreateBuffer(
        envExrImage.imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent
    );
    void* mapped = stagingMemory.mapMemory(0, envExrImage.imageSize);
    memcpy(mapped, envExrImage.pixelData.data(), envExrImage.imageSize);
    stagingMemory.unmapMemory();

    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        // High precision color format to prevent color compression and color banding
        .format = vk::Format::eR32G32B32A32Sfloat,
        .extent = {
            envExrImage.width,
            envExrImage.height,
            1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eSampled |
            vk::ImageUsageFlagBits::eTransferDst,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
    };

    m_envImage = vk::raii::Image(device, imageInfo);

    vk::MemoryRequirements memRequirements = m_envImage.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vk_util::FindMemoryType(
            memRequirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            physicalDevice
        )
    };
    m_envImageMemory = vk::raii::DeviceMemory(device, allocInfo);
    m_envImage.bindMemory(m_envImageMemory, 0);

    vk::ImageViewCreateInfo viewInfo{
        .image = *m_envImage,
        .viewType = vk::ImageViewType::e2D,
        // Must match the image's format exactly since the image object has an immutable format
        .format = vk::Format::eR32G32B32A32Sfloat,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    m_envImageView = vk::raii::ImageView(device, viewInfo);

    vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice.getProperties();
    vk::SamplerCreateInfo samplerInfo{
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eClampToEdge,
        .addressModeV = vk::SamplerAddressMode::eClampToEdge,
        .addressModeW = vk::SamplerAddressMode::eClampToEdge,
        .anisotropyEnable = vk::False,
        // .maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy
    };

    m_envImageSampler = vk::raii::Sampler(device, samplerInfo);

    vk::CommandBufferAllocateInfo cmdInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    vk::raii::CommandBuffer commandBuffer = std::move(device.allocateCommandBuffers(cmdInfo).front());
    commandBuffer.begin({
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    });

    vk_util::TransitionImageLayoutGeneric(
        *m_envImage,
        commandBuffer,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        {},
        vk::AccessFlagBits2::eTransferWrite,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        vk::PipelineStageFlagBits2::eAllTransfer
    );

    vk::BufferImageCopy region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {envExrImage.width, envExrImage.height, 1}
    };
    commandBuffer.copyBufferToImage(
        *stagingBuffer,
        *m_envImage,
        vk::ImageLayout::eTransferDstOptimal,
        region
    );

    vk_util::TransitionImageLayoutGeneric(
        *m_envImage,
        commandBuffer,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::AccessFlagBits2::eTransferWrite,
        vk::AccessFlagBits2::eShaderSampledRead,
        vk::PipelineStageFlagBits2::eAllTransfer,
        vk::PipelineStageFlagBits2::eComputeShader
    );

    commandBuffer.end();

    queue.submit(vk::SubmitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer
    }, nullptr);
    // Wait until the info is submitted
    queue.waitIdle();
}
