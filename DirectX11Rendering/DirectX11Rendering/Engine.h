#pragma once
#include "EngineBase.h"

extern bool g_bUsePerspectiveProjection;
extern bool g_bUseDrawNormals;
extern bool g_bUseDrawWireFrame;

class Engine : public EngineBase
{
public:
	Engine();
	virtual ~Engine();

public:
	virtual bool Initialize() override;

public:
	virtual void Update(float dt) override;
	virtual void UpdateGUI() override;
	virtual void Render() override;

public:
	const std::shared_ptr<Camera> GetMainCamera() { return m_mainCamera; }

	vector<std::shared_ptr<Light>>& GetLights() { return m_lights; }

private:
	vector<std::shared_ptr<Mesh>> m_meshes;
	vector<std::shared_ptr<Light>> m_lights;
	std::shared_ptr<Camera> m_mainCamera;
};
