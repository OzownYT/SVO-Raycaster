#include "Camera.hpp"

#include <iostream>

Camera::Camera(glm::vec3 position, glm::vec3 up, CmData data) : m_Position(position), m_WorldUp(up), m_Data(data) {}

void Camera::Update(glm::ivec3 direction, glm::vec2 mouse) {
    static float lastX = mouse.x / 10, lastY = mouse.y / 10;
    static bool first = true;
    if (direction.x == 1)
        m_Position += m_Right * m_Data.Speed;
    if (direction.x == -1)
        m_Position -= m_Right * m_Data.Speed;
    if (direction.y == 1)
        m_Position += m_WorldUp * m_Data.Speed;
    if (direction.y == -1)
        m_Position -= m_WorldUp * m_Data.Speed;
    if (direction.z == 1)
        m_Position += m_Forward * m_Data.Speed;
    if (direction.z == -1)
        m_Position -= m_Forward * m_Data.Speed;

    mouse *= m_Data.Sensitivity;

    m_Data.Yaw += mouse.x - lastX;
    m_Data.Pitch += lastY - mouse.y;
    lastX = mouse.x;
    lastY = mouse.y;

    if (m_Data.Pitch > 89.0f) m_Data.Pitch = 89.0f;
    if (m_Data.Pitch < -89.0f) m_Data.Pitch = -89.0f;
    UpdateCameraVectors();
}

void Camera::Zoom(float offset) {
    m_Data.Zoom -= offset * 2;
    if (m_Data.Zoom < 1.0f)
        m_Data.Zoom = 1.0f;
    if (m_Data.Zoom > 75.0f)
        m_Data.Zoom = 75.0f;
    m_Projection = glm::perspective(glm::radians(m_Data.Zoom), m_Aspect, m_Near, m_Far);
}

void Camera::UpdateCameraVectors() {
    glm::vec3 newForward;
    newForward.x = cos(glm::radians(m_Data.Yaw)) * cos(glm::radians(m_Data.Pitch));
    newForward.y = sin(glm::radians(m_Data.Pitch));
    newForward.z = sin(glm::radians(m_Data.Yaw)) * cos(glm::radians(m_Data.Pitch));
    m_Forward = newForward;

    m_Right = glm::normalize(glm::cross(m_Forward, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
}
