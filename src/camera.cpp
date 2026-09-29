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

    m_targetFov = m_fov;
    m_baseFov = m_fov;
    m_zoomSmoothing = 20.0f;

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

void Camera::SetTargetZoom(float scrollOffset) {
    float zoomFac = scrollOffset * m_scrollSensitivity;
    m_targetFov -= glm::radians(zoomFac);
    m_targetFov = glm::clamp(m_targetFov, m_minFov, m_maxFov);
}

void Camera::UpdateZoom(float deltaTime) {
    float t = 1.0f - std::exp(-m_zoomSmoothing * deltaTime);
    m_fov = glm::mix(m_fov, m_targetFov, t);
}
