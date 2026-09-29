#pragma once


#include <glm/glm.hpp>


class Camera {
public:
    void Init();

    void UpdateVectors();

    void ClampFov();

    glm::vec3 m_pos;
    float m_yaw;
    float m_pitch;
    glm::vec3 m_right;
    glm::vec3 m_up;
    glm::vec3 m_front;
    float m_fov;
    float m_moveSensitivity;
    float m_scrollSensitivity;

private:
    glm::vec3 m_worldUp;
    float m_minFov;
    float m_maxFov;
};
