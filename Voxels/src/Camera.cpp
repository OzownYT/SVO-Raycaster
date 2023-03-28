#include "Camera.hpp"

#include <iostream>

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up)
    : m_Position(position),
      m_WorldUp(up),
      m_Data(CmData{}),
      m_Mode(CameraMode::FREE) {}

void Camera::Update(glm::ivec3 direction, glm::vec2 mouse) {
    static float lastX = mouse.x / 10, lastY = mouse.y / 10;
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

    if (m_Mode == CameraMode::FREE) {
        UpdateFree();
        m_Data.Yaw += mouse.x - lastX;
        m_Data.Pitch += lastY - mouse.y;
        lastX = mouse.x;
        lastY = mouse.y;
    } else if (m_Mode == CameraMode::ORBITAL)
        UpdateOrbital();

    if (m_Data.Pitch > 89.0f) m_Data.Pitch = 89.0f;
    if (m_Data.Pitch < -89.0f)
        m_Data.Pitch = -89.0f;
}

void Camera::Zoom(float offset) {
    m_Data.Zoom -= offset * 2;
    if (m_Data.Zoom < 1.0f)
        m_Data.Zoom = 1.0f;
    if (m_Data.Zoom > 75.0f)
        m_Data.Zoom = 75.0f;
    m_Projection = glm::perspective(glm::radians(m_Data.Zoom), m_Aspect, m_Near, m_Far);
}

void Camera::UpdateFree() {
    glm::vec3 newForward;
    newForward.x = cos(glm::radians(m_Data.Yaw)) * cos(glm::radians(m_Data.Pitch));
    newForward.y = sin(glm::radians(m_Data.Pitch));
    newForward.z = sin(glm::radians(m_Data.Yaw)) * cos(glm::radians(m_Data.Pitch));
    m_Forward = newForward;

    m_Right = glm::normalize(glm::cross(m_Forward, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
}

void Camera::UpdateOrbital() {
    m_Right = {};
    m_Up = {};
    m_Forward = {};
    m_Right.x = 500 * cos(glm::radians(m_Data.Yaw)) * cos(glm::radians(m_Data.Pitch));
    m_Up.y = 500 * sin(glm::radians(m_Data.Pitch));
    m_Forward.z = 500 * cos(glm::radians(m_Data.Pitch)) * sin(glm::radians(m_Data.Yaw));
}
