#pragma once
#include "Object.h"
#include <SimpleMath.h>

using DirectX::SimpleMath::Vector3;

struct LightData
{
	Vector3 strength = Vector3(0.0f);
	float fallOffStart = 0.0f;
	Vector3 direction = Vector3(0.0f, -1.0f, 0.0f);
	float fallOffEnd = 0.0f;
	Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
	float spotPower = 1.0f;
};

class Light : public Object
{
public:
	Light(LightData& lightData) :m_lightData(lightData) {}

public:
	virtual void Update(float dt) override;

public:
	LightData& GetLightData() { return m_lightData; }

protected:
	LightData m_lightData;
};
