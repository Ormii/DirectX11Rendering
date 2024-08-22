#pragma once

class ProxyModel : public Model
{
public:
	void ProxyModeInitialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context, const String& basePath,
		const String& filename);
	void ProxyModeInitialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context,
		Vector<MeshData> meshes);


	virtual void Render(ComPtr<ID3D11DeviceContext>& context) override;

private:
	Vector<shared_ptr<Mesh>> m_proxyMeshes;
};

