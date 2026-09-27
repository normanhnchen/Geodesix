#include <vector>
#include <random>

#include <glm/glm.hpp>

#include "buffer_data.hpp"


namespace buffer_data {


namespace particle {


/**
 * @see https://docs.vulkan.org/tutorial/latest/11_Compute_Shader.html
 */
std::vector<Particle> GenerateInitialParticles(uint32_t width, uint32_t height) {
    // Initialize particles
    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

    // Initial particle positions on a circle
    std::vector<Particle> particles(PARTICLE_COUNT);
    for (auto& particle : particles) {
        float r = 0.25f * sqrtf(rndDist(rndEngine));
        float theta = rndDist(rndEngine) * 2.0f * 3.14159265358979323846f;
        float x = r * cosf(theta) * height / width;
        float y = r * sinf(theta);
        particle.position = glm::vec2(x, y);
        particle.velocity = normalize(glm::vec2(x,y)) * 0.00025f;
        particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
    }

    return particles;
}


} // namespace particle


namespace vertex {


const std::vector<Vertex> vertices = {
    // {{Position}, {Texture coordinates}}
    {{-1.0f, -1.0f}, {0.0f, 0.0f}},
    {{ 1.0f, -1.0f}, {1.0f, 0.0f}},
    {{ 1.0f,  1.0f}, {1.0f, 1.0f}},
    {{-1.0f,  1.0f}, {0.0f, 1.0f}}
};


} // namespace vertex


namespace index {


const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};


} // namespace index


} // namespace buffer_data
