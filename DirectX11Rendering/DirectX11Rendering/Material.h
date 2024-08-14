#pragma once
#include <SimpleMath.h>

using DirectX::SimpleMath::Vector3;

struct Material
{
	Vector3 ambient = Vector3(0.1f);
	float shininess = 1.0f;
	Vector3 diffuse = Vector3(1.0f);
	float dummy1;
	Vector3 specular = Vector3(1.0f);
	float dummy2;
};