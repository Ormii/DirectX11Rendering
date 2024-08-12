#pragma once
#include "Object.h"
#include <SimpleMath.h>

using DirectX::SimpleMath::Vector3;

enum class LightType
{
	LightType_DirectionalLight	= 0,
	LightType_PointLight		= 1,
	LightType_SpotLight			= 2,
};

struct LightData
{
	LightType lightType = LightType::LightType_PointLight;
	float	dummy1;
	float	dummy2;
	float	dummy3;

	Vector3 strength = Vector3(1.0f);
	float fallOffStart = 0.0f;
	Vector3 direction = Vector3(0.0f, 0.0f, 1.0f);
	float fallOffEnd = 0.0f;
	Vector3 position = Vector3(0.0f, 0.0f, -2.0f);
	float spotPower = 1.0f;
};

class Light : public Object
{
public:
	Light(LightData& lightData) :m_lightData(lightData) {}

public:
	LightData& GetLightData() { return m_lightData; }

protected:
	LightData& m_lightData;
};
