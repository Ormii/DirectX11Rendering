#pragma once

struct ModelVertexConstantData {
    Matrix model;
    Matrix invTranspose;
    Matrix view;
    Matrix projection;
};

struct ModelPixelConstantData {
    Vector3 eyeWorld;         
    bool useTexture;          
    Material material;        
    LightData directionalLight;
    LightData pointlight[MAX_LIGHTS];
    LightData spotlight[MAX_LIGHTS];
};


class Model : public Object
{
public:
    void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context, const std::string& basePath,
        const std::string& filename);

    void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context,
        const std::vector<MeshData>& meshes);

    void UpdateConstantBuffers(ComPtr<ID3D11Device>& device,
        ComPtr<ID3D11DeviceContext>& context);

    void Render(ComPtr<ID3D11DeviceContext>& context);

public:
    virtual void Update(float dt) override;

protected:
    virtual void CreateSamplerState(ComPtr<ID3D11Device>& device);

protected:
    ModelVertexConstantData m_modelVertexConstantData;
    ModelPixelConstantData m_modelPixelConstantData;

    ComPtr<ID3D11ShaderResourceView> m_diffuseResView;
    ComPtr<ID3D11ShaderResourceView> m_specularResView;


private:
    std::vector<shared_ptr<Mesh>> m_meshes;

    ComPtr<ID3D11VertexShader> m_modelVertexShader;
    ComPtr<ID3D11PixelShader> m_modelPixelShader;
    ComPtr<ID3D11InputLayout> m_modelInputLayout;

    ComPtr<ID3D11Buffer> m_modelvertexConstantBuffer;
    ComPtr<ID3D11Buffer> m_modelpixelConstantBuffer;

private:
    ComPtr<ID3D11SamplerState> m_modelDefaultSamplerState;
};

