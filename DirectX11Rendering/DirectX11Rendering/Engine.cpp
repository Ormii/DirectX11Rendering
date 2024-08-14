#include "pch.h"
#include "Engine.h"

bool g_bUsePerspectiveProjection = true;
bool g_bUseDrawNormals = false;
bool g_bUseDrawWireFrame = false;

ThreadManager<Engine>* g_ThreadManager = nullptr;

Engine::Engine()
{
	g_ThreadManager = new ThreadManager<Engine>();
}

Engine::~Engine()
{
	delete g_ThreadManager;
}

bool Engine::Initialize()
{
	if(!EngineBase::Initialize())
		return false;

	m_mainCamera = std::make_shared<Camera>();
	m_mainCamera->GetTranslation() = Vector3(0.0f, 0.0f, 10.0);

	auto zelda = make_shared<Model>();
	zelda->Initialize(m_device, m_context, "../Resources/zelda/", "zeldaPosed001.fbx");
	zelda->GetTranslation() = Vector3(0.0f, 0.0f, -3.5f);
	zelda->GetScaling() = Vector3(4.0f, 4.0f, 4.0f);
	m_models.push_back(zelda);

	auto floor = make_shared<Model>();
	floor->Initialize(m_device, m_context, vector<MeshData>{GeometryGenerator::MakeBox(1.0f)});
	floor->GetTranslation() = Vector3(0.0f, -3.0f, 0.0f);
	floor->GetRotation() = Vector3(3.14f / 2, 0.0f, 0.0f);
	floor->GetScaling() = Vector3(10.0f, 10.0f, 1.0f);
	m_models.push_back(floor);

	LightData lightData{};
	m_directionalLight = std::make_shared<Light>(lightData);
	m_directionalLight->GetDirection() = Vector3(-1.0f, -1.0f, -1.0);
	m_directionalLight->GetDirection().Normalize();
	m_directionalLight->GetLightData().strength = Vector3(0.5f);

	for (int32 i = 0; i < MAX_LIGHTS; ++i)
	{
		LightData pointLightData{};
		m_pointLights.push_back(std::make_shared<Light>(pointLightData));
	}

	for (int32 i = 0; i < MAX_LIGHTS; ++i)
	{
		LightData spotLightData{};
		m_spotLights.push_back(std::make_shared<Light>(spotLightData));
	}

	m_pointLights[0]->GetLightData().strength = Vector3(1.0f);
	m_pointLights[0]->GetTranslation() = Vector3(0.0f, 3.5f, 0.0f);
	m_pointLights[0]->GetLightData().fallOffStart = 3.0f;
	m_pointLights[0]->GetLightData().fallOffEnd = 8.0f;

	m_spotLights[0]->GetLightData().strength = Vector3(1.0f);
	m_spotLights[0]->GetTranslation() = Vector3(0.0f, 3.5f, 0.0f);
	m_spotLights[0]->GetDirection() = Vector3(0.0f, -1.0f, 0.0f);
	m_spotLights[0]->GetLightData().fallOffStart = 3.0f;
	m_spotLights[0]->GetLightData().fallOffEnd = 8.0f;

	m_targetModel = zelda;
	m_targetLight = m_spotLights[0];

	m_commandLists.resize(1);

	return true;
}

void Engine::Update(float dt)
{
	m_mainCamera->Update(dt);

	m_directionalLight->Update(dt);

	for (auto& light : m_pointLights)
	{
		light->Update(dt);
	}

	for (auto& light : m_spotLights)
	{
		light->Update(dt);
	}

	g_ThreadManager->Launch(&Engine::UpdateMeshes, this, ThreadParam{ 0,  dt});

	g_ThreadManager->Join();
}

void Engine::UpdateGUI()
{
	ImGui::Checkbox("Draw WireFrame", &g_bUseDrawWireFrame);
	ImGui::Checkbox("Draw Normals", &g_bUseDrawNormals);
	ImGui::Checkbox("Use Perspective", &g_bUsePerspectiveProjection);

	if (!m_targetModel.expired())
	{
		std::shared_ptr<Model> targetModel = m_targetModel.lock();
		ImGui::SliderFloat3("TargetMesh Translation", &targetModel->GetTranslation().x, -10.0f, 10.0f);
		ImGui::SliderFloat3("TargetMesh Rotation", &targetModel->GetRotation().x, -3.14f, 3.14f);
		ImGui::SliderFloat3("TargetMesh Scaling", &targetModel->GetScaling().x, 0.1f, 10.0f);
	}

	if (!m_targetLight.expired())
	{
		std::shared_ptr<Light> targetLight = m_targetLight.lock();
		ImGui::SliderFloat3("Target Light Position", &targetLight->GetTranslation().x, -10.0f, 10.0f);
		ImGui::SliderFloat3("Target Light Direction", &targetLight->GetDirection().x, -1.0, 1.0f);
		ImGui::SliderFloat3("Target Light Strength", &targetLight->GetLightData().strength.x, 0.0f, 1.0f);
		ImGui::SliderFloat("Target Light FallOffStart", &targetLight->GetLightData().fallOffStart, 0.0f, 5.0f);
		ImGui::SliderFloat("Target Light FallOffEnd", &targetLight->GetLightData().fallOffEnd, 0.0f, 10.0f);
		ImGui::SliderFloat("Target Light SpotPower", &targetLight->GetLightData().spotPower, 0.0f, 512.0f);
	}
}

void Engine::Render()
{
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f, 0);

	for (int i = 0; i < 1; ++i)
	{
		ThreadParam MeshRenderThreadParam{};
		MeshRenderThreadParam.commandListIdx = i;

		g_ThreadManager->Launch(&Engine::RenderMeshes, this, MeshRenderThreadParam);
	}

	g_ThreadManager->Join();

	for (auto& commandList : m_commandLists) {
		m_context->ExecuteCommandList(commandList.Get(), FALSE);
	}

	m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get());
	m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
}

void Engine::UpdateMeshes(ThreadParam param)
{
	for (auto& mesh : m_models)
	{
		mesh->Update(param.deltatime);
		mesh->UpdateConstantBuffers(m_device, m_context);
	}
}

void Engine::RenderMeshes(ThreadParam param)
{
	ComPtr<ID3D11DeviceContext> deferredContext;

	m_lock.lock();
	m_device->CreateDeferredContext(0, &deferredContext);
	m_lock.unlock();

	deferredContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get());
	deferredContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(m_screenWidth);  // 뷰포트의 너비
	viewport.Height = static_cast<float>(m_screenHeight); // 뷰포트의 높이
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	deferredContext->RSSetViewports(1, &viewport);

	for (auto& model : m_models)
	{
		model->Render(deferredContext);
	}

	ComPtr<ID3D11CommandList> commandList;
	deferredContext->FinishCommandList(FALSE, &commandList);

	m_lock.lock();
	m_commandLists[param.commandListIdx] = commandList;
	m_lock.unlock();
}
