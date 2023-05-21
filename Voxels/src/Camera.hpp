#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMode {
	FREE,
	ORBITAL
};

enum class CmMove {
	Forward,
	Backward,
	Left,
	Right
};

struct CmData {
	float Pitch = 0.0f, Yaw = -90.f;
	float Speed = 2.5f, Sensitivity = 0.1f;
	float Zoom = 75.f;
};

class Camera {
public:
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.f, 3.f), glm::vec3 target = glm::vec3(0.f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));

	void SetProjection(float fov, float aspect, float near, float far) {
		m_Aspect = aspect, m_Near = near, m_Far = far;
		m_Projection = glm::perspective(glm::radians(fov), aspect, near, far);
	}

	glm::mat4 GetProjection() { return m_Projection; }
	glm::mat4 GetView() { return glm::lookAt(m_Position, m_Position + m_Forward, m_Up); }
	glm::mat4 GetView(glm::vec3 target) { return glm::lookAt(m_Position, target, m_Up); }
	glm::vec3 GetPosition() { return m_Position; }
	glm::vec3 GetViewDirection() { return glm::normalize(m_Forward); }

	void Update(glm::ivec3 direction, glm::vec2 mouse);
	void Zoom(float offset);
	void SetSpeed(float speed) { m_Data.Speed = speed; }
	void SetMode(CameraMode mode) { m_Mode = mode; }
	void SetPosition(glm::vec3 pos) { m_Position = pos; }

private:
	void UpdateFree();
	void UpdateOrbital();

private:
	CameraMode m_Mode;
	float m_Aspect, m_Near, m_Far;
	glm::mat4 m_Projection;
	glm::vec3 m_Position, m_Up, m_Forward, m_Right, m_WorldUp;
	CmData m_Data;
};
