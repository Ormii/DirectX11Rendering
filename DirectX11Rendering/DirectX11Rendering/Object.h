#pragma once

class Object
{

public:
	Vector3& GetTranslation() { return m_modelTranslation; }
	Vector3& GetRotation() { return m_modelRotation; }
	Vector3& GetScaling() { return m_modelScaling; }

protected:
	Vector3 m_modelTranslation = Vector3(0.0f);
	Vector3 m_modelRotation = Vector3(0, 0.0f, 0.0f);
	Vector3 m_modelScaling = Vector3(1.0f);
};

