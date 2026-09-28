#include "camera.hpp"


void Camera::Init() {
    m_yaw = 90.0f;
    m_pitch = 0.0f;

    m_worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
    
    UpdateVectors();

    m_fov = 45.0f;
    
    m_sensitivity = 0.1f;
}

void Camera::UpdateVectors() {
    m_front.x = cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
    m_front.y = cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));
    m_front.z = sin(glm::radians(m_pitch));
    m_front = glm::normalize(m_front);

    m_right = glm::normalize(glm::cross(m_front, m_worldUp));

    m_up = glm::normalize(glm::cross(m_right, m_front));
}
