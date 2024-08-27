#include "pch.h"
#include "Camera.h"
#include <d3d11.h>
Camera::Camera()
{
	
}


void Camera::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context, float aspect)
{
	m_aspect = aspect;

	MeshData meshData{};

	GeometryGenerator::MakeFrustom("PerspectiveFrustom", m_aspect, XMConvertToRadians(m_projFovAngleY), m_nearZ, m_farZ, meshData);
	m_frustomMesh = MakeShared<Mesh>();

	vector<D3D11_INPUT_ELEMENT_DESC> inputElements =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,4 * 3,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,4 * 3 + 4 * 3,D3D11_INPUT_PER_VERTEX_DATA,0},
	};

	EngineUtility::CreateVertexShaderAndInputLayout(device, context, L"BoundingVertexShader.hlsl", inputElements, m_frustomVertexShader, m_frustomInputLayout);
	EngineUtility::CreatePixelShader(device, context, L"BoundingPixelShader.hlsl", m_frustomPixelShader);

	EngineUtility::CreateConstantBuffer(device, context, m_cameraVertexConstantBufferData, m_frustomMesh->vertexConstantBuffer);

	m_frustomMesh->m_indexCount = (UINT)meshData.indices.size();
	EngineUtility::CreateVertexBuffer(device, context, meshData.vertices, m_frustomMesh->vertexBuffer);
	EngineUtility::CreateIndexBuffer(device, context, meshData.indices, m_frustomMesh->indexBuffer);

	auto bindFunc = std::bind(&Camera::CreateSamplerState, this, std::placeholders::_1);
	SamplerGenerator::MakeSampler("CameraFrustomSampler", m_frustomSamplerState, bindFunc, device);
}

void Camera::Update(float dt)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;
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

void Camera::UpdateConstantBuffers(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	auto modelRow =
		Matrix::CreateRotationZ(-GetRotation().z)
		* Matrix::CreateRotationX(-GetRotation().x)
		* Matrix::CreateRotationY(GetRotation().y)
		* Matrix::CreateTranslation(GetTranslation());

	auto invTransposeRow = modelRow.Transpose();
	invTransposeRow.Translation(Vector3(0.0f));
	invTransposeRow = invTransposeRow.Transpose().Invert();

	auto viewRow = pEngine->GetMainCamera()->GetViewMat();

	Matrix projRow = Matrix();

	if (g_bUsePerspectiveProjection)
		projRow = pEngine->GetMainCamera()->GetPersMat();
	else
		projRow = pEngine->GetMainCamera()->GetOrthMat();

	m_cameraVertexConstantBufferData.model = modelRow.Transpose();
	m_cameraVertexConstantBufferData.view = viewRow.Transpose();
	m_cameraVertexConstantBufferData.invTranspose = invTransposeRow;
	m_cameraVertexConstantBufferData.projection = projRow.Transpose();

	EngineUtility::UpdateBuffer(device, context, m_cameraVertexConstantBufferData, m_frustomMesh->vertexConstantBuffer);
}

void Camera::Render(ComPtr<ID3D11DeviceContext>& context)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	if (g_bUseDrawCameraFrustom)
	{
		if (pEngine->GetMainCamera().get() != this)
		{
			UINT stride = sizeof(Vertex), offset = 0;
			{
				context->RSSetState(pEngine->GetWiredRasterizerState().Get());

				context->IASetVertexBuffers(0, 1, m_frustomMesh->vertexBuffer.GetAddressOf(), &stride, &offset);
				context->IASetInputLayout(m_frustomInputLayout.Get());
				context->IASetIndexBuffer(m_frustomMesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, offset);
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


				context->VSSetConstantBuffers(0, 1, m_frustomMesh->vertexConstantBuffer.GetAddressOf());
				context->VSSetShader(m_frustomVertexShader.Get(), 0, 0);

				context->PSSetShader(m_frustomPixelShader.Get(), 0, 0);
				context->PSSetSamplers(0, 1, m_frustomSamplerState.GetAddressOf());
				context->DrawIndexed(m_frustomMesh->m_indexCount, 0, 0);

				if (!g_bUseDrawWireFrame)
					context->RSSetState(pEngine->GetSolidRasterizerState().Get());
			}
		}
	}
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
	return Matrix::CreateTranslation(-m_translation) *
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

ComPtr<ID3D11SamplerState> Camera::CreateSamplerState(ComPtr<ID3D11Device> device)
{
	ComPtr<ID3D11SamplerState> samplerState;
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());

	return samplerState;
}

