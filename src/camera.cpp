#include "camera.hpp"


void Camera::Init() {
    m_pos = glm::vec3(0.0f, 0.0f, 0.0f);

    m_yaw = 90.0f;
    m_pitch = 0.0f;

    m_worldUp = glm::vec3(0.0f, 0.0f, 1.0f);

    UpdateVectors();

    m_fov = glm::radians(45.0f);
    
    m_moveSensitivity = 0.1f;
    m_scrollSensitivity = 1.0f;

    m_minFov = glm::radians(1.0f);
    m_maxFov = glm::radians(115.0f);
}

void Camera::UpdateVectors() {
    m_front.x = cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
    m_front.y = cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));
    m_front.z = sin(glm::radians(m_pitch));
    m_front = glm::normalize(m_front);

    m_right = glm::normalize(glm::cross(m_front, m_worldUp));

    m_up = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::ClampFov() {
    if (m_fov < m_minFov) {
        m_fov = m_minFov;
    }
    if (m_fov > m_maxFov) {
        m_fov = m_maxFov;
    }
}
