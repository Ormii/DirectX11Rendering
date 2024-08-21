#include "pch.h"
#include "Model.h"
#include "GeometryGenerator.h"

void Model::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context, const String& basePath, const String& filename)
{
	Vector<MeshData> meshes;
	GeometryGenerator::ReadFromFile(basePath, filename, meshes);

	Initialize(device, context, meshes);
}

void Model::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context, const Vector<MeshData>& meshes)
{
	m_modelVertexConstantData.model = Matrix();
	m_modelVertexConstantData.view = Matrix();
	m_modelVertexConstantData.projection = Matrix();

	CreateSamplerState(device);

	EngineUtility::CreateConstantBuffer(device, context, m_modelVertexConstantData, m_modelvertexConstantBuffer);
	EngineUtility::CreateConstantBuffer(device, context, m_modelPixelConstantData, m_modelpixelConstantBuffer);

	int32 totVerticesCount = 0;
	Vector3 minPoint = Vector3(10000, 10000, 10000);
	Vector3 maxPoint = Vector3(-10000, -10000, -10000);
	for (const auto& meshData : meshes)
	{
		auto newmesh = MakeShared<Mesh>();
		EngineUtility::CreateVertexBuffer(device, context, meshData.vertices, newmesh->vertexBuffer);
		EngineUtility::CreateIndexBuffer(device, context, meshData.indices, newmesh->indexBuffer);

		if (!meshData.textureFilename.empty())
		{
			EngineUtility::CreateTexture(device, meshData.textureFilename, newmesh->texture, newmesh->textureResourceView);
		}

		newmesh->vertexConstantBuffer = m_modelvertexConstantBuffer;
		newmesh->pixelConstantBuffer = m_modelpixelConstantBuffer;
		newmesh->m_indexCount = static_cast<UINT>(meshData.indices.size());
		totVerticesCount += meshData.vertices.size();
		this->m_meshes.push_back(newmesh);

		m_boundingDefaultRadius = 0;
		m_boundingDefaultCenter = Vector3(0.0f, 0.0f, 0.0f);
		for (auto& obj : meshData.vertices)
		{
			minPoint.x = min(obj.position.x, minPoint.x);
			minPoint.y = min(obj.position.y, minPoint.y);
			minPoint.z = min(obj.position.z, minPoint.z);
			maxPoint.x = max(obj.position.x, maxPoint.x);
			maxPoint.y = max(obj.position.y, maxPoint.y);
			maxPoint.z = max(obj.position.z, maxPoint.z);
		}
	}

	m_boundingDefaultCenter = (minPoint + maxPoint)/2.0f;
	m_boundingDefaultRadius = (maxPoint - minPoint).Length()/2.0f;


	vector<D3D11_INPUT_ELEMENT_DESC> inputElements =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,4*3,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,4*3+4*3,D3D11_INPUT_PER_VERTEX_DATA,0},
	};

	EngineUtility::CreateVertexShaderAndInputLayout(device, context, L"ModelVertexShader.hlsl", inputElements, m_modelVertexShader, m_modelInputLayout);
	EngineUtility::CreatePixelShader(device, context, L"ModelPixelShader.hlsl", m_modelPixelShader);

	/*
	
	m_normalLines = std::make_shared<Mesh>();

	std::vector<Vertex> normalVertices;
	std::vector<uint32_t> normalIndices;

	size_t offset = 0;
	for (const auto& meshData : meshes) {
		for (size_t i = 0; i < meshData.vertices.size(); i++) {

			auto v = meshData.vertices[i];

			v.texcoord.x = 0.0f;
			normalVertices.push_back(v);

			v.texcoord.x = 1.0f;
			normalVertices.push_back(v);

			normalIndices.push_back(uint32_t(2 * (i + offset)));
			normalIndices.push_back(uint32_t(2 * (i + offset) + 1));
		}
		offset += meshData.vertices.size();
	}

	EngineUtility::CreateVertexBuffer(device, context, normalVertices,
		m_normalLines->vertexBuffer);
	m_normalLines->m_indexCount = UINT(normalIndices.size());
	EngineUtility::CreateIndexBuffer(device, context, normalIndices,
		m_normalLines->indexBuffer);

	EngineUtility::CreateVertexShaderAndInputLayout(
		device, context, L"NormalVertexShader.hlsl", inputElements,
		m_normalVertexShader, m_modelInputLayout);
	EngineUtility::CreatePixelShader(device, context, L"NormalPixelShader.hlsl",
		m_normalPixelShader);

	EngineUtility::CreateConstantBuffer(device, context, m_normalVertexConstantData,
		m_normalVertexConstantBuffer);
	*/

	m_boundingMesh = MakeShared<Mesh>();
	MeshData boundingMeshData{};
	GeometryGenerator::MakeSphere("ModelBoundingSphere", m_boundingSphere.Radius, 20, 10, boundingMeshData);
	m_boundingMesh->m_indexCount = (UINT)boundingMeshData.indices.size();
	EngineUtility::CreateVertexBuffer(device, context, boundingMeshData.vertices, m_boundingMesh->vertexBuffer);
	EngineUtility::CreateIndexBuffer(device, context, boundingMeshData.indices, m_boundingMesh->indexBuffer);

	EngineUtility::CreateVertexShaderAndInputLayout(device, context, L"BoundingVertexShader.hlsl", inputElements, m_boundingVertexShader,m_boundingInputLayout);
	EngineUtility::CreatePixelShader(device, context, L"BoundingPixelShader.hlsl", m_boundingPixelShader);
}

void Model::UpdateConstantBuffers(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	for (const auto& mesh : m_meshes)
	{
		auto modelRow = Matrix::CreateScale(GetScaling())
			* Matrix::CreateRotationY(GetRotation().y)
			* Matrix::CreateRotationX(GetRotation().x)
			* Matrix::CreateRotationZ(GetRotation().z)
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
		
		auto eyeWorld = Vector3::Transform(Vector3(0.0f), viewRow.Invert());

		m_modelPixelConstantData.directionalLight = pEngine->GetDirectionalLight()->GetLightData();
		for (int i = 0; i < MAX_LIGHTS; ++i)
		{
			m_modelPixelConstantData.pointlight[i] = pEngine->GetPointLights()[i]->GetLightData();
			m_modelPixelConstantData.spotlight[i] = pEngine->GetSpotLights()[i]->GetLightData();
		}

		m_modelVertexConstantData.model = modelRow.Transpose();
		m_modelVertexConstantData.view = viewRow.Transpose();
		m_modelVertexConstantData.invTranspose = invTransposeRow;
		m_modelVertexConstantData.projection = projRow.Transpose();

		m_modelPixelConstantData.eyeWorld = eyeWorld;
		m_modelPixelConstantData.material = mesh->mat;
		
		EngineUtility::UpdateBuffer(device, context, m_modelVertexConstantData, mesh->vertexConstantBuffer);
		EngineUtility::UpdateBuffer(device, context, m_modelPixelConstantData, mesh->pixelConstantBuffer);
	}
	/*

	if (g_bUseDrawNormals)
	{
		m_normalVertexConstantData.scale = 1.0f;
		EngineUtility::UpdateBuffer(device, context, m_normalVertexConstantData,
			m_normalVertexConstantBuffer);
	}
	*/
}

void Model::Render(ComPtr<ID3D11DeviceContext>& context)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;
	
	if (!pEngine->GetFrustomCullingCamera()->CheckBoundingSphereInFrustom(m_boundingSphere, 
		Matrix::CreateTranslation(GetTranslation())))
		return;
	

	context->VSSetShader(m_modelVertexShader.Get(), 0, 0);
	context->PSSetSamplers(0, 0, m_modelDefaultSamplerState.GetAddressOf());
	context->PSSetShader(m_modelPixelShader.Get(), 0, 0);

	if (g_bUseDrawWireFrame)
		context->RSSetState(pEngine->GetWiredRasterizerState().Get());
	else
		context->RSSetState(pEngine->GetSolidRasterizerState().Get());

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

	{
		context->RSSetState(pEngine->GetWiredRasterizerState().Get());

		context->IASetVertexBuffers(0, 1, m_boundingMesh->vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetInputLayout(m_boundingInputLayout.Get());
		context->IASetIndexBuffer(m_boundingMesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, offset);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		context->VSSetConstantBuffers(0, 1, m_modelvertexConstantBuffer.GetAddressOf());
		context->VSSetShader(m_boundingVertexShader.Get(), 0, 0);

		context->PSSetShader(m_boundingPixelShader.Get(), 0, 0);
		context->DrawIndexed(m_boundingMesh->m_indexCount, 0, 0);

		if(!g_bUseDrawWireFrame)
			context->RSSetState(pEngine->GetSolidRasterizerState().Get());
	}



	/*
	if (g_bUseDrawNormals)
	{
		context->VSSetShader(m_normalVertexShader.Get(), 0, 0);
		ID3D11Buffer* pptr[2] = { m_modelvertexConstantBuffer.Get(),
								 m_normalVertexConstantBuffer.Get() };
		context->VSSetConstantBuffers(0, 2, pptr);
		context->PSSetShader(m_normalPixelShader.Get(), 0, 0);
		context->IASetInputLayout(m_modelInputLayout.Get());
		context->IASetVertexBuffers(
			0, 1, m_normalLines->vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(m_normalLines->indexBuffer.Get(),
			DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		context->DrawIndexed(m_normalLines->m_indexCount, 0, 0);
	}
	*/
}

void Model::Update(float dt)
{
	Vector3 temp = Vector3(m_boundingDefaultRadius, 0, 0);
	Vector3 scaled = Vector3::Transform(temp, Matrix::CreateScale(GetScaling()));
	m_boundingSphere.Radius = scaled.Length();
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