#pragma once
#include <SimpleMath.h>

using DirectX::SimpleMath::Vector3;

struct Light
{
	Vector3 strength = Vector3(1.0f);
	float fallOffStart = 0.0f;
	Vector3 direction = Vector3(0.0f, 0.0f, 1.0f);
	float fallOffEnd = 0.0f;
	Vector3 position = Vector3(0.0f, 0.0f, -2.0f);
	float spotPower = 1.0f;
};