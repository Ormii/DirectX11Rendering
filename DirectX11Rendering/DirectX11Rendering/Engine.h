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
	virtual void Update() override;
	virtual void Render() override;

public:
	const std::shared_ptr<Camera> GetMainCamera() { return m_mainCamera; }

private:
	vector<std::shared_ptr<Mesh>> meshes;
	std::shared_ptr<Camera> m_mainCamera;
};
