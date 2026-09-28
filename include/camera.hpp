#pragma once


#include <glm/glm.hpp>


class Camera {
public:
    void Init();

    void UpdateVectors();

    glm::vec3 m_pos;
    float m_yaw;
    float m_pitch;
    glm::vec3 m_right;
    glm::vec3 m_up;
    glm::vec3 m_front;
    float m_fov;
    float m_sensitivity;

private:
    glm::vec3 m_worldUp;
};
