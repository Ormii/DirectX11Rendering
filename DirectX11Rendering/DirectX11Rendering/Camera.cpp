#include "pch.h"
#include "Camera.h"

Camera::Camera()
{
	
}

void Camera::Update(float dt)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	m_aspect = pEngine->GetAspectRatio();

	float pitch = m_rotation.x;
	float yaw = m_rotation.y;
	float roll = m_rotation.z;

	auto rotationMatrix = Matrix::CreateFromYawPitchRoll(yaw, -pitch, roll);

	Vector3 Look = Vector3(0.0f, 0.0f, 1.0f);
	Look = Vector3::Transform(Look, rotationMatrix);
	Look.Normalize();

	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	up = Vector3::Transform(up, rotationMatrix);
	up.Normalize();

	Vector3 target = Vector3(m_translation.x + Look.x,
		m_translation.y + Look.y,
		m_translation.z + Look.z);
		
	m_viewMatrix = Matrix::CreateLookAt(m_translation, target, up);

	m_rightDir = Vector3(m_viewMatrix.m[0][0], m_viewMatrix.m[1][0], m_viewMatrix.m[2][0]);
	m_upDir = Vector3(m_viewMatrix.m[0][1], m_viewMatrix.m[1][1], m_viewMatrix.m[2][1]);
	m_forwardDir = Vector3(m_viewMatrix.m[0][2], m_viewMatrix.m[1][2], m_viewMatrix.m[2][2]);
}

void Camera::UpdateMouse(float deltaMouseNdcX, float deltaMouseNdcY)
{
	m_rotation.y += deltaMouseNdcX * DirectX::XM_2PI;
	m_rotation.x += -deltaMouseNdcY * DirectX::XM_PIDIV2;
}

void Camera::MoveForward(float dt)
{
	m_translation += m_forwardDir * m_moveSpeed * dt * (m_bSpeedUp ? 3.0f : 1.0f);
}

void Camera::MoveRight(float dt)
{
	m_translation += m_rightDir * m_moveSpeed * dt * (m_bSpeedUp ? 3.0f : 1.0f);
}

Matrix Camera::GetViewMat()
{
	return m_viewMatrix;
}

Matrix Camera::GetPersMat()
{
	return XMMatrixPerspectiveFovLH(XMConvertToRadians(m_projFovAngleY), m_aspect,
		m_nearZ, m_farZ);
}

Matrix Camera::GetOrthMat()
{
	return XMMatrixOrthographicOffCenterLH(-m_aspect, m_aspect, -1.0f, 1.0f, m_nearZ, m_farZ);
}

