#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Camera {
public:
    Camera(float fov, float aspectRatio, float nearClip, float farClip);

    void SetPosition(const glm::vec3& position);
    void SetRotation(const glm::vec2& rotation);

    const glm::vec3& GetPosition() const;
    const glm::vec2& GetRotation() const;
    const glm::vec3& GetForward() const;
    const glm::vec3& GetRight() const;
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    void SetProjection(float fov, float aspectRatio, float nearClip, float farClip);
    void SetAspectRatio(float aspectRatio);

    void UpdateView();

 private:
    float m_FOV, m_Aspect, m_Near, m_Far;
    glm::vec3 m_Position;
    glm::vec2 m_Rotation; 
    glm::vec3 m_Forward, m_Right, m_Up;

    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ProjectionMatrix;

    void RecalculateVectors();
};

