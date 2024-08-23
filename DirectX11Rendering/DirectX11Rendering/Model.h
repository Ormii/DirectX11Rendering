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

struct ModelHullConstantBuffer{
    Vector3 eyeWorld;
    float weight;

    bool useLod;
    float dummy[3];
};



class Model : public Object
{
public:
    void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context, const String& basePath,
        const String& filename, bool useLod = true, bool drawBoundingArea = true);

    void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context,
        const Vector<MeshData>& meshes, bool useLod = true, bool drawBoundingArea = true);

    void UpdateConstantBuffers(ComPtr<ID3D11Device>& device,
        ComPtr<ID3D11DeviceContext>& context);

    virtual void Render(ComPtr<ID3D11DeviceContext>& context);

public:
    virtual void Update(float dt) override;

    void SetDiffuseResView(ComPtr<ID3D11ShaderResourceView> diffuseResView) { m_diffuseResView = diffuseResView; }
    void SetSpecularResView(ComPtr<ID3D11ShaderResourceView> specularResView) { m_specularResView = specularResView; }

protected:
    virtual ComPtr<ID3D11SamplerState> CreateSamplerState(ComPtr<ID3D11Device> device);

protected:
    ModelVertexConstantData m_modelVertexConstantData;
    ModelPixelConstantData m_modelPixelConstantData;

    ComPtr<ID3D11ShaderResourceView> m_diffuseResView;
    ComPtr<ID3D11ShaderResourceView> m_specularResView;


protected:
    Vector<shared_ptr<Mesh>> m_meshes;

    ComPtr<ID3D11VertexShader> m_modelVertexShader;
    ComPtr<ID3D11PixelShader> m_modelPixelShader;
    ComPtr<ID3D11InputLayout> m_modelInputLayout;

    ComPtr<ID3D11Buffer> m_modelvertexConstantBuffer;
    ComPtr<ID3D11Buffer> m_modelpixelConstantBuffer;
protected:

    ModelHullConstantBuffer m_modelHullConstantData;

    ComPtr<ID3D11HullShader>   m_modelHullShader;
    ComPtr<ID3D11DomainShader> m_modelDomainShader;
    ComPtr<ID3D11Buffer> m_modelHullConstantBuffer;

protected:

    ComPtr<ID3D11VertexShader> m_normalVertexShader;
    ComPtr<ID3D11PixelShader> m_normalPixelShader;

    shared_ptr<Mesh> m_normalLines;

    NormalVertexConstantData m_normalVertexConstantData;
    ComPtr<ID3D11Buffer> m_normalVertexConstantBuffer;
    ComPtr<ID3D11Buffer> m_normalPixelConstantBuffer;


protected:
    BoundingSphere           m_boundingSphere;
    Vector3                  m_boundingDefaultCenter;
    float                    m_boundingDefaultRadius;
    shared_ptr<Mesh> m_boundingMesh;
    ComPtr<ID3D11VertexShader> m_boundingVertexShader;
    ComPtr<ID3D11PixelShader> m_boundingPixelShader;
    ComPtr<ID3D11InputLayout> m_boundingInputLayout;

protected:
    ComPtr<ID3D11SamplerState> m_modelDefaultSamplerState;

protected:
    bool m_bDrawBoundingArea;
};

