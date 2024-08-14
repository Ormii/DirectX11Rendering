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


private:
	vector<ComPtr<ID3D11CommandList>> m_commandLists;

	void UpdateMeshes(ThreadParam param);

	void RenderMeshes(ThreadParam param);

	mutex m_lock;
public:
	const std::shared_ptr<Camera> GetMainCamera() { return m_mainCamera; }

	vector<std::shared_ptr<Light>>& GetLights() { return m_lights; }

private:
	vector<std::shared_ptr<Model>> m_models;
	vector<std::shared_ptr<Light>> m_lights;
	std::shared_ptr<Camera> m_mainCamera;

private:
	std::weak_ptr<Model> m_targetModel;
	std::weak_ptr<Light> m_targetLight;

};

extern ThreadManager<Engine>* g_ThreadManager;
