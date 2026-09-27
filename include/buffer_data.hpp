#pragma once


#include <array>
#include <cstddef>
#include <ctime>
#include <cmath>

#include "header_inclusions/vulkan.hpp"

#include <glm/glm.hpp>


/**
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/00_Vertex_input_description.html
 */
struct Vertex {
    glm::vec2 inPos;
    glm::vec2 inTexCoords;

    /**
     * The binding states the index in the array of bindings.
     * 
     * The stride states how many bytes it takes from one entry to the next.
     * 
     * The input rate can be either of the following values:
     * 
     * - vk::VertexInputRate::eVertex:
     *      Move from vertex to vertex
     * 
     * - vk::VertexInputRate::eInstance:
     *      Move from vertex 
     */
    static vk::VertexInputBindingDescription getBindingDescription() {
        return {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = vk::VertexInputRate::eVertex
        };
    }

    /**
     * The location represents the location of the input in the vertex shader.
     * 
     * The binding states which binding the per-vertex data comes.
     * 
     * The format states the type of data for the attribute.
     */
    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
        return {{
            {
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = offsetof(Vertex, inPos)
            },
            {
                .location = 1,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = offsetof(Vertex, inTexCoords)
            }
        }};
    }
};


namespace buffer_data {


namespace vertex {


extern const std::vector<Vertex> vertices;


} // namespace vertex


namespace index {


extern const std::vector<uint16_t> indices;


}


namespace uniform {


struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct ComputeUniformBufferObject {
    float deltaTime = 1.0f;
};


} // namespace uniform


namespace particle {


struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;
};

constexpr uint32_t PARTICLE_COUNT = 16384 * 256;

std::vector<Particle> GenerateInitialParticles(uint32_t width, uint32_t height);

constexpr std::size_t PARTICLES_BUFFER_SIZE = sizeof(Particle) * PARTICLE_COUNT;


} // namespace particle


} // namespace buffer_data
