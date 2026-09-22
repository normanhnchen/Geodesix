#pragma once


#include <array>
#include <cstddef>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <glm/glm.hpp>


/**
 * @see https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/00_Vertex_input_description.html
 */
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

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
                .offset = offsetof(Vertex, pos)
            },
            {
                .location = 1,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = offsetof(Vertex, color)
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


} // namespace buffer_data
