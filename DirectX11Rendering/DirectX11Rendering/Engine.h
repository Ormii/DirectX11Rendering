#pragma once
#include "EngineBase.h"

extern bool g_bUsePerspectiveProjection;
extern bool g_bUseDrawNormals;
extern bool g_bUseDrawWireFrame;


#define MAX_MODEL_ROW_NUM	20
#define MAX_MODEL_COL_NUM	20

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
	void SetLights();



private:
	Vector<ComPtr<ID3D11CommandList>> m_commandLists;

	void UpdateWithThread(float dt);

	bool UpdateMeshesThread(ThreadParam param, promise<bool>&&);
	bool UpdateCubeMapThread(ThreadParam param, promise<bool>&&);
	bool UpdateFloorThread(ThreadParam param, promise<bool>&&);

	void RenderWithThread();

	bool RenderMeshesThread(ThreadParam param, promise<bool>&&);
	bool RenderCubMapThread(ThreadParam param, promise<bool>&&);
	bool RenderFloorThread(ThreadParam param, promise<bool>&&);

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
	shared_ptr<ProxyModel> MakeZelda();
	shared_ptr<ProxyModel> MakeDuck();
	shared_ptr<ProxyModel> MakeMonster();
	shared_ptr<ProxyModel> MakeCorset();
private:


	Vector<std::shared_ptr<Model>> m_models;

	shared_ptr<Light> m_directionalLight;
	Vector<std::shared_ptr<Light>> m_pointLights;
	Vector<std::shared_ptr<Light>> m_spotLights;
	
	Vector<std::shared_ptr<Camera>> m_cameras;

	std::shared_ptr<CubeMap> m_cubeMap;
	std::shared_ptr<Model> m_floor;

	int32 m_mainCameraIdx = 0;

private:
	Vector<std::shared_ptr<ImageFilter>> m_imageFilters;

	float m_threshold = 0.1f;
	float m_strength = 0.1f;
	float m_lodWeight = 1.0f;
	float m_proxyMeshAppliedDist = 10.0f;
};
