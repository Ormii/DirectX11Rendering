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
	virtual void KeyBeginPress(WPARAM wParam) override;
	virtual void KeyEndPress(WPARAM wParam) override;
private:
	void LoadResources();

private:
	Vector<ComPtr<ID3D11CommandList>> m_commandLists;

	bool UpdateMeshes(ThreadParam param, promise<bool>&&);
	bool UpdateCubeMap(ThreadParam param, promise<bool>&&);

	bool RenderMeshes(ThreadParam param, promise<bool>&&);
	bool RenderCubMap(ThreadParam param, promise<bool>&&);

	Lock m_deviceLock;
	Lock m_contextLock;
	Lock m_commandListLock;
	Lock m_modelLock;

public:
	const std::shared_ptr<Camera> GetMainCamera() { return m_cameras[m_mainCameraIdx]; }
	const std::shared_ptr<Camera> GetFrustomCullingCamera() { return m_cameras[0]; }

	shared_ptr<Light>& GetDirectionalLight(){ return m_directionalLight; }
	Vector<std::shared_ptr<Light>>& GetPointLights() { return m_pointLights; }
	Vector<std::shared_ptr<Light>>& GetSpotLights() { return m_spotLights; }

	float GetLodWeight() { return m_lodWeight; }
	float GetProxyAppliedDistance() { return m_proxyMeshAppliedDist; }
private:


	Vector<std::shared_ptr<Model>> m_models;

	shared_ptr<Light> m_directionalLight;
	Vector<std::shared_ptr<Light>> m_pointLights;
	Vector<std::shared_ptr<Light>> m_spotLights;
	
	Vector<std::shared_ptr<Camera>> m_cameras;

	std::shared_ptr<CubeMap> m_cubeMap;

	int32 m_mainCameraIdx = 0;

private:
	std::weak_ptr<Model> m_targetModel;
	std::weak_ptr<Light> m_targetLight;

private:
	Vector<std::shared_ptr<ImageFilter>> m_imageFilters;

	float m_threshold = 0.1f;
	float m_strength = 0.1f;
	float m_lodWeight = 1.0f;
	float m_proxyMeshAppliedDist = 10.0f;
};
