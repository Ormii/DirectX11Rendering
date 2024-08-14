#include "pch.h"
#include "Model.h"
#include "GeometryGenerator.h"

void Model::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context, const std::string& basePath, const std::string& filename)
{
	vector<MeshData> meshes = GeometryGenerator::ReadFromFile(basePath, filename);
	Initialize(device, context, meshes);
}

void Model::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context, const std::vector<MeshData>& meshes)
{
	m_modelVertexConstantData.model = Matrix();
	m_modelVertexConstantData.view = Matrix();
	m_modelVertexConstantData.projection = Matrix();

	CreateSamplerState(device);

	EngineUtility::CreateConstantBuffer(device, context, m_modelVertexConstantData, m_modelvertexConstantBuffer);
	EngineUtility::CreateConstantBuffer(device, context, m_modelPixelConstantData, m_modelpixelConstantBuffer);

	for (const auto& meshData : meshes)
	{
		auto newmesh = make_shared<Mesh>();
		EngineUtility::CreateVertexBuffer(device, context, meshData.vertices, newmesh->vertexBuffer);
		EngineUtility::CreateIndexBuffer(device, context, meshData.indices, newmesh->indexBuffer);

		if (!meshData.textureFilename.empty())
		{
			EngineUtility::CreateTexture(device, meshData.textureFilename, newmesh->texture, newmesh->textureResourceView);
		}

		newmesh->vertexConstantBuffer = m_modelvertexConstantBuffer;
		newmesh->pixelConstantBuffer = m_modelpixelConstantBuffer;
		newmesh->m_indexCount = meshData.indices.size();

		this->m_meshes.push_back(newmesh);
	}

	vector<D3D11_INPUT_ELEMENT_DESC> inputElements =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
	};

	EngineUtility::CreateVertexShaderAndInputLayout(device, context, L"ModelVertexShader.hlsl", inputElements, m_modelVertexShader, m_modelInputLayout);
	EngineUtility::CreatePixelShader(device, context, L"ModelPixelShader.hlsl", m_modelPixelShader);
}

void Model::UpdateConstantBuffers(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	for (const auto& mesh : m_meshes)
	{
		m_modelVertexConstantData.model = Matrix::CreateScale(GetScaling())
			* Matrix::CreateRotationX(GetRotation().x)
			* Matrix::CreateRotationY(GetRotation().y)
			* Matrix::CreateRotationZ(GetRotation().z)
			* Matrix::CreateTranslation(GetTranslation());

		m_modelVertexConstantData.invTranspose = m_modelVertexConstantData.model;
		m_modelVertexConstantData.invTranspose.Translation(Vector3(0.0));
		m_modelVertexConstantData.invTranspose = m_modelVertexConstantData.invTranspose.Invert().Transpose();

		m_modelVertexConstantData.model = m_modelVertexConstantData.model.Transpose();

		m_modelVertexConstantData.view = Matrix::CreateRotationX(pEngine->GetMainCamera()->GetRotation().x)
			* Matrix::CreateRotationY(pEngine->GetMainCamera()->GetRotation().y)
			* Matrix::CreateTranslation(pEngine->GetMainCamera()->GetTranslation());

		m_modelVertexConstantData.view = m_modelVertexConstantData.view.Transpose();

		const float aspect = pEngine->GetAspectRatio();
		Matrix projMat = Matrix();
		float nearZ = pEngine->GetMainCamera()->GetNearZ(), farZ = pEngine->GetMainCamera()->GetFarZ();

		if (g_bUsePerspectiveProjection)
			projMat = XMMatrixPerspectiveFovLH(XMConvertToRadians(pEngine->GetMainCamera()->GetFovAngle()), aspect,
				nearZ, farZ);
		else
			projMat = XMMatrixOrthographicOffCenterLH(-aspect, aspect, -1.0f, 1.0f, nearZ, farZ);
		
		m_modelVertexConstantData.projection = projMat.Transpose();

		int32 pointLightIdx = 0, spotLightIdx = 0;


		m_modelPixelConstantData.eyeWorld = Vector3::Transform(Vector3(0.0f), m_modelVertexConstantData.view);

		for (int i = 0; i < MAX_LIGHTS && i < pEngine->GetLights().size(); ++i)
		{
			weak_ptr<Light> pWeakLight = pEngine->GetLights()[i];
			if (pWeakLight.expired())
				continue;

			shared_ptr<Light> pLight = pWeakLight.lock();

			switch (pLight->GetLightData().lightType)
			{
				case LightType::LightType_DirectionalLight:
					m_modelPixelConstantData.directionalLight = pLight->GetLightData();
					break;
				case LightType::LightType_PointLight:
					m_modelPixelConstantData.pointlight[pointLightIdx++] = pLight->GetLightData();
					break;
				case LightType::LightType_SpotLight:
					m_modelPixelConstantData.pointlight[spotLightIdx++] = pLight->GetLightData();
					break;
				default:
					break;
			}
		}

		m_modelPixelConstantData.material = mesh->mat;
		
		EngineUtility::UpdateBuffer(device, context, m_modelVertexConstantData, mesh->vertexConstantBuffer);
		EngineUtility::UpdateBuffer(device, context, m_modelPixelConstantData, mesh->pixelConstantBuffer);
	}
}

void Model::Render(ComPtr<ID3D11DeviceContext>& context)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	context->VSSetShader(m_modelVertexShader.Get(), 0, 0);
	context->PSSetSamplers(0, 0, m_modelDefaultSamplerState.GetAddressOf());
	context->PSSetShader(m_modelPixelShader.Get(), 0, 0);

	UINT stride = sizeof(Vertex), offset = 0;
	for (auto& mesh : m_meshes)
	{
		context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetInputLayout(m_modelInputLayout.Get());
		context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		context->VSSetConstantBuffers(0, 1, mesh->vertexConstantBuffer.GetAddressOf());
		context->PSSetConstantBuffers(0, 1, mesh->pixelConstantBuffer.GetAddressOf());

		ID3D11ShaderResourceView* resViews[3] = 
		{
			mesh->textureResourceView.Get(), m_diffuseResView.Get(),
			m_specularResView.Get()
		};

		context->PSSetShaderResources(0, 3, resViews);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->DrawIndexed(mesh->m_indexCount, 0, 0);
	}
}

void Model::Update(float dt)
{

}

void Model::CreateSamplerState(ComPtr<ID3D11Device>& device)
{
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, m_modelDefaultSamplerState.GetAddressOf());
}