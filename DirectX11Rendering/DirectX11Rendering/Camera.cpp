#include "pch.h"
#include "Camera.h"

Camera::Camera()
{
	m_forwardDir = Vector3(0.0f, 0.0f, 1.0f);
	m_rightDir = Vector3(1.0f, 0.0f, 0.0f);
	m_upDir = Vector3(0.0f, 1.0f, 0.0f);
}

void Camera::Update(float dt)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	m_aspect = pEngine->GetAspectRatio();

	m_rotation.x = m_pitch;
	m_rotation.y = -m_yaw;
}

void Camera::UpdateMouse(float mouseNdcX, float mouseNdcY)
{
	m_yaw = mouseNdcX * DirectX::XM_2PI;
	m_pitch = mouseNdcY * DirectX::XM_PIDIV2;

	m_forwardDir = Vector3::Transform(Vector3(0.0f, 0.0f, -1.0f), Matrix::CreateRotationY(m_yaw));
	m_rightDir = m_upDir.Cross(m_forwardDir);
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
	return Matrix::CreateTranslation(m_translation)
		* Matrix::CreateRotationY(m_rotation.y)
		* Matrix::CreateRotationX(m_rotation.x);
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

