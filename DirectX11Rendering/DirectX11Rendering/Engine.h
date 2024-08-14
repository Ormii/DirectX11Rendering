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

	shared_ptr<Light>& GetDirectionalLight(){ return m_directionalLight; }
	vector<std::shared_ptr<Light>>& GetPointLights() { return m_pointLights; }
	vector<std::shared_ptr<Light>>& GetSpotLights() { return m_spotLights; }
private:
	vector<std::shared_ptr<Model>> m_models;

	shared_ptr<Light> m_directionalLight;
	vector<std::shared_ptr<Light>> m_pointLights;
	vector<std::shared_ptr<Light>> m_spotLights;
	std::shared_ptr<Camera> m_mainCamera;

private:
	std::weak_ptr<Model> m_targetModel;
	std::weak_ptr<Light> m_targetLight;

};

extern ThreadManager<Engine>* g_ThreadManager;
