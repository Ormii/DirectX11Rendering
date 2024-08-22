#include "pch.h"
#include "ProxyModel.h"

void ProxyModel::ProxyModeInitialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context, const String& basePath, const String& filename)
{
	Vector<MeshData> meshes;
	GeometryGenerator::ReadFromFile(basePath, filename, meshes);

	ProxyModeInitialize(device, context, meshes);
}

void ProxyModel::ProxyModeInitialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context, Vector<MeshData> meshes)
{
	for (auto& meshData : meshes)
	{
		int32 step = 0;
		int32 i = 0;

		int32 indicesSize = meshData.indices.size() / 3;
		for (i = 0, step = 0; i < indicesSize && step < meshData.indices.size(); i+=3, step += 6)
		{
			meshData.indices[i] = meshData.indices[step];
			meshData.indices[i+1] = meshData.indices[step+1];
			meshData.indices[i+2] = meshData.indices[step+2];
		}
		meshData.indices.resize(indicesSize);

		auto newmesh = MakeShared<Mesh>();
		EngineUtility::CreateVertexBuffer(device, context, meshData.vertices, newmesh->vertexBuffer);
		EngineUtility::CreateIndexBuffer(device, context, meshData.indices, newmesh->indexBuffer);

		if (!meshData.textureFilename.empty())
		{
			EngineUtility::CreateTexture(device, meshData.textureFilename, newmesh->texture, newmesh->textureResourceView);
		}

		newmesh->vertexConstantBuffer = m_modelvertexConstantBuffer;
		newmesh->pixelConstantBuffer = m_modelpixelConstantBuffer;
		newmesh->hullConstantBuffer = m_modelHullConstantBuffer;
		newmesh->m_indexCount = static_cast<UINT>(meshData.indices.size());

		this->m_proxyMeshes.push_back(newmesh);
	}
}

void ProxyModel::Render(ComPtr<ID3D11DeviceContext>& context)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	float dist = (pEngine->GetMainCamera()->GetTranslation() - GetTranslation()).Length();

	if (pEngine->GetProxyAppliedDistance() > dist)
	{
		Model::Render(context);
		return;
	}


	if (!pEngine->GetFrustomCullingCamera()->CheckBoundingSphereInFrustom(m_boundingSphere,
		Matrix::CreateTranslation(GetTranslation())))
		return;


	context->VSSetShader(m_modelVertexShader.Get(), 0, 0);
	context->PSSetSamplers(0, 1, m_modelDefaultSamplerState.GetAddressOf());
	context->PSSetShader(m_modelPixelShader.Get(), 0, 0);

	if (g_bUseDrawWireFrame)
		context->RSSetState(pEngine->GetWiredRasterizerState().Get());
	else
		context->RSSetState(pEngine->GetSolidRasterizerState().Get());

	UINT stride = sizeof(Vertex), offset = 0;
	for (auto& mesh : m_proxyMeshes)
	{
		context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetInputLayout(m_modelInputLayout.Get());
		context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		context->VSSetConstantBuffers(0, 1, mesh->vertexConstantBuffer.GetAddressOf());

		context->HSSetConstantBuffers(0, 1, mesh->hullConstantBuffer.GetAddressOf());
		context->HSSetShader(m_modelHullShader.Get(), 0, 0);

		context->DSSetConstantBuffers(0, 1, mesh->vertexConstantBuffer.GetAddressOf());
		context->DSSetShader(m_modelDomainShader.Get(), 0, 0);

		context->PSSetConstantBuffers(0, 1, mesh->pixelConstantBuffer.GetAddressOf());

		ID3D11ShaderResourceView* resViews[3] =
		{
			mesh->textureResourceView.Get(), m_diffuseResView.Get(),
			m_specularResView.Get()
		};

		context->PSSetShaderResources(0, 3, resViews);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		context->DrawIndexed(mesh->m_indexCount, 0, 0);

		context->HSSetShader(nullptr, 0, 0);
		context->DSSetShader(nullptr, 0, 0);
	}

}
