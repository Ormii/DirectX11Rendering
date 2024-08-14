#include "pch.h"
#include "Light.h"

void Light::Update(float dt)
{
	m_lightData.position = m_translation;
	m_lightData.direction = m_direction;
}
