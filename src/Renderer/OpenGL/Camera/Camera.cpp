#include "Camera.h"
#include <glm/gtc/constants.hpp>


Camera::Camera(float fov, float aspect, float nearClip, float farClip)
    : m_FOV(fov), m_Aspect(aspect), m_Near(nearClip), m_Far(farClip),
      m_Position(0.0f), m_Rotation(0.0f, -90.0f) {
    m_ProjectionMatrix = glm::perspective(glm::radians(fov), aspect, nearClip, farClip);
    RecalculateVectors();
    UpdateView();
}
void Camera::SetPosition(const glm::vec3& position) {
    m_Position = position;
    UpdateView();
}
void Camera::SetRotation(const glm::vec2& rotation) {
    m_Rotation = rotation;
    RecalculateVectors();
    UpdateView();
}
const glm::vec3& Camera::GetPosition() const { return m_Position; }
const glm::vec2& Camera::GetRotation() const { return m_Rotation; }
const glm::vec3& Camera::GetForward() const { return m_Forward; }
const glm::vec3& Camera::GetRight() const { return m_Right; }
glm::mat4 Camera::GetViewMatrix() const { return m_ViewMatrix; }
glm::mat4 Camera::GetProjectionMatrix() const { return m_ProjectionMatrix; }

    void Camera::SetProjection(float fov, float aspectRatio, float nearClip, float farClip) {
    m_FOV = fov;
    m_Aspect = aspectRatio;
    m_Near = nearClip;
    m_Far = farClip;
    m_ProjectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
}
void Camera::SetAspectRatio(float aspectRatio) {
    m_Aspect = aspectRatio;
    m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_Aspect, m_Near, m_Far);
}
void Camera::RecalculateVectors() {
    float pitch = glm::radians(m_Rotation.x);
    float yaw   = glm::radians(m_Rotation.y);
    m_Forward = glm::normalize(glm::vec3(
        cos(pitch) * cos(yaw),
        sin(pitch),
        cos(pitch) * sin(yaw)
    ));
    m_Right = glm::normalize(glm::cross(m_Forward, glm::vec3(0, 1, 0)));
    m_Up    = glm::normalize(glm::cross(m_Right, m_Forward));
}
void Camera::UpdateView() {
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
}

