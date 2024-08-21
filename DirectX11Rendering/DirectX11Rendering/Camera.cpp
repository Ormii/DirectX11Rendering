#include "pch.h"
#include "Camera.h"
#include <d3d11.h>
Camera::Camera()
{
	
}

void Camera::Update(float dt)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	m_aspect = pEngine->GetAspectRatio();

	float pitch = -m_rotation.x;
	float yaw = m_rotation.y;
	float roll = m_rotation.z;

	auto rotationMatrix = Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);

	Vector3 Look = Vector3(0.0f, 0.0f, 1.0f);
	Look = Vector3::Transform(Look, rotationMatrix);
	Look.Normalize();

	Vector3 Right = Vector3::Up.Cross(Look);
	Right.Normalize();

	Vector3 Up = Look.Cross(Right);
	Up.Normalize();

	Vector3 target = Vector3(m_translation.x + Look.x,
		m_translation.y + Look.y,
		m_translation.z + Look.z);
		
	//m_viewMatrix = Matrix::CreateLookAt(m_translation, target, up);

	m_rightDir = Right;
	m_upDir = Up;
	m_forwardDir = Look;

	ConstructFrustomPlanes();
}

void Camera::UpdateMouse(float deltaMouseNdcX, float deltaMouseNdcY)
{
	m_rotation.y += deltaMouseNdcX * DirectX::XM_2PI;
	m_rotation.x += deltaMouseNdcY * DirectX::XM_PIDIV2;
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
	return Matrix::CreateTranslation(-m_translation)*
		Matrix::CreateRotationY(-m_rotation.y)*
		Matrix::CreateRotationX(m_rotation.x)*
		Matrix::CreateRotationZ(m_rotation.z);
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

void Camera::ConstructFrustomPlanes()
{
	Matrix projMat = GetOrthMat();
	if (g_bUsePerspectiveProjection)
		projMat = GetPersMat();


	Matrix viewMat = GetViewMat();
	
	Matrix viewProj = viewMat * projMat;

	// Left
	m_planes[0].x = viewProj._14 + viewProj._11;
	m_planes[0].y = viewProj._24 + viewProj._21;
	m_planes[0].z = viewProj._34 + viewProj._31;
	m_planes[0].w = viewProj._44 + viewProj._41;

	// Right
	m_planes[1].x = viewProj._14 - viewProj._11;
	m_planes[1].y = viewProj._24 - viewProj._21;
	m_planes[1].z = viewProj._34 - viewProj._31;
	m_planes[1].w = viewProj._44 - viewProj._41;

	// Bottom
	m_planes[2].x = viewProj._14 + viewProj._12;
	m_planes[2].y = viewProj._24 + viewProj._22;
	m_planes[2].z = viewProj._34 + viewProj._32;
	m_planes[2].w = viewProj._44 + viewProj._42;

	// Top
	m_planes[3].x = viewProj._14 - viewProj._12;
	m_planes[3].y = viewProj._24 - viewProj._22;
	m_planes[3].z = viewProj._34 - viewProj._32;
	m_planes[3].w = viewProj._44 - viewProj._42;

	// Near
	m_planes[4].x = viewProj._14 + viewProj._13;
	m_planes[4].y = viewProj._24 + viewProj._23;
	m_planes[4].z = viewProj._34 + viewProj._33;
	m_planes[4].w = viewProj._44 + viewProj._43;

	// Far
	m_planes[5].x = viewProj._14 - viewProj._13;
	m_planes[5].y = viewProj._24 - viewProj._23;
	m_planes[5].z = viewProj._34 - viewProj._33;
	m_planes[5].w = viewProj._44 - viewProj._43;

	for (int i = 0; i < 6; i++)
	{
		float length = sqrtf(m_planes[i].x * m_planes[i].x + m_planes[i].y * m_planes[i].y + m_planes[i].z * m_planes[i].z);
		m_planes[i].x /= length;
		m_planes[i].y /= length;
		m_planes[i].z /= length;
		m_planes[i].w /= length;
	}
	
}

bool Camera::CheckBoundingSphereInFrustom(BoundingSphere& boundingSphere, Matrix&& World)
{
	Vector3 sphereCenter = Vector3(0.0f,0.0f,0.0f);
	sphereCenter = Vector3::Transform(sphereCenter, World );
	

	for (uint32 i = 0; i < 6; ++i)
	{
		if (m_planes[i].Dot(Vector4(sphereCenter.x, sphereCenter.y, sphereCenter.z,1.0f)) < -boundingSphere.Radius)
		{
			return false;
		}
	}

	return true;
}

