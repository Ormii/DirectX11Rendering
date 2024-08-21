#pragma once
#include "Object.h"
#include <SimpleMath.h>

using DirectX::SimpleMath::Vector3;

class Camera : public Object
{
public:
    Camera();

public:
    virtual void Update(float dt) override;
    void UpdateMouse(float mouseNdcX, float mouseNdcY);

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
    float m_farZ = 1000.0f;

    float m_moveSpeed = 3.0f;
    bool m_bSpeedUp = false;

    float m_yaw;
    float m_pitch;

    Vector3 m_forwardDir;
    Vector3 m_rightDir;
    Vector3 m_upDir;

    Matrix m_viewMatrix;

    Vector4 m_planes[6];
};

