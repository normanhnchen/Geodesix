#include <vector>

#include <glm/glm.hpp>

#include "buffer_data.hpp"


namespace buffer_data {


namespace vertex {


const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};


} // namespace vertex


namespace index {


const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};


} // namespace index


namespace uniform {


} // namespace uniform


} // namespace buffer_data
