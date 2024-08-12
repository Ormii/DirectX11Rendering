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

public:
    float& GetFovAngle() { return m_projFovAngleY; }
    float& GetNearZ() { return m_nearZ; }
    float& GetFarZ() { return m_farZ; }

private:

    float m_projFovAngleY = 70.0f;
    float m_nearZ = 0.01f;
    float m_farZ = 100.0f;
};

