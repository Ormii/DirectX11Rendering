#pragma once
#include "Object.h"
#include <SimpleMath.h>
#include "Mesh.h"

using DirectX::SimpleMath::Vector3;

struct CameraVertexConstantData {
    Matrix model;
    Matrix invTranspose;
    Matrix view;
    Matrix projection;
};

class Camera : public Object
{
public:
    Camera();

public:
    void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext> context, float aspect);
    virtual void Update(float dt) override;
    void UpdateMouse(float mouseNdcX, float mouseNdcY);
    void UpdateConstantBuffers(ComPtr<ID3D11Device>& device,
        ComPtr<ID3D11DeviceContext>& context);

    void Render(ComPtr<ID3D11DeviceContext>& context);
public:
    void MoveForward(float dt);
    void MoveRight(float dt);

    void SetSpeedUp(bool speedUp) { m_bSpeedUp = speedUp; }

public:
    float& GetFovAngle() { return m_projFovAngleY; }
    float& GetNearZ() { return m_nearZ; }
    float& GetFarZ() { return m_farZ; }

public:
    Matrix GetViewMat();
    Matrix GetPersMat();
    Matrix GetOrthMat();

public:
    void ConstructFrustomPlanes();
    bool CheckBoundingSphereInFrustom(BoundingSphere& boundingSphere, Matrix&& World);
    
private:
    float m_aspect = 0.0f;

    float m_projFovAngleY = 70.0;
    float m_nearZ = 0.01f;
    float m_farZ = 100.0f;

    float m_moveSpeed = 3.0f;
    bool m_bSpeedUp = false;

    float m_yaw;
    float m_pitch;

    Vector3 m_forwardDir;
    Vector3 m_rightDir;
    Vector3 m_upDir;

    Matrix m_viewMatrix;
private:
    Vector4 m_planes[6];

    shared_ptr<Mesh> m_frustomMesh;

    CameraVertexConstantData m_cameraVertexConstantBufferData;

    ComPtr<ID3D11VertexShader> m_frustomVertexShader;
    ComPtr<ID3D11PixelShader> m_frustomPixelShader;
    ComPtr<ID3D11InputLayout> m_frustomInputLayout;

    virtual ComPtr<ID3D11SamplerState> CreateSamplerState(ComPtr<ID3D11Device> device);

    ComPtr<ID3D11SamplerState> m_frustomSamplerState;
};

