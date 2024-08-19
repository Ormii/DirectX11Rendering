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


protected:
	virtual void OnMouseMove(WPARAM wParam, int mouseX, int mouseY) override;

private:
	void LoadResources();

private:
	vector<ComPtr<ID3D11CommandList>> m_commandLists;

	void UpdateMeshes(ThreadParam param);
	void UpdateCubeMap(ThreadParam param);

	void RenderMeshes(ThreadParam param);
	void RenderCubMap(ThreadParam param);

	Lock m_deviceLock;
	Lock m_contextLock;
	Lock m_commandListLock;
	Lock m_modelLock;

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
	std::shared_ptr<CubeMap> m_cubeMap;

private:
	std::weak_ptr<Model> m_targetModel;
	std::weak_ptr<Light> m_targetLight;

private:
	vector<std::shared_ptr<ImageFilter>> m_imageFilters;

	float m_threshold = 0.1f;
	float m_strength = 0.1f;
};

extern ThreadManager<Engine>* g_ThreadManager;
