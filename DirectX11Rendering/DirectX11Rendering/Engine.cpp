#include "pch.h"
#include "Engine.h"

bool g_bUsePerspectiveProjection = true;
bool g_bUseDrawNormals = true;
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
	m_mainCamera->GetTranslation() = Vector3(0.0f, 0.0f, 10.0f);

	auto floor = make_shared<Model>();
	floor->Initialize(m_device, m_context, vector<MeshData>{GeometryGenerator::MakeSquare()});

	floor->GetTranslation() = Vector3(0.0f, -5.0f, 8.0f);
	floor->GetScaling() = Vector3(10.0f, 10.0f, 1.0f);
	floor->GetRotation() = Vector3(3.14f/2, 0.0f, 0.0f);

	m_models.push_back(floor);

	LightData lightData{};
	lightData.lightType = LightType::LightType_DirectionalLight;
	auto directionalLight = std::make_shared<Light>(lightData);

	LightData pointLightData{};
	pointLightData.lightType = LightType::LightType_PointLight;
	auto pointLight = std::make_shared<Light>(pointLightData);
	pointLight->GetTranslation() = Vector3(0.0f, 0.0f, 7.25f);
	pointLight->GetLightData().fallOffStart = 3.5f;
	pointLight->GetLightData().fallOffEnd = 8.5f;

	LightData spotLightData{};
	spotLightData.lightType = LightType::LightType_SpotLight;
	auto spotLight = std::make_shared<Light>(spotLightData);
	spotLight->GetLightData().fallOffStart = 3.2f;
	spotLight->GetLightData().fallOffEnd = 8.5f;
	spotLight->GetLightData().spotPower = 9.0f;
	spotLight->GetTranslation() = Vector3(7.3f, 1.75f, 4.4f);
	spotLight->GetDirection() = Vector3(-0.3f, -1.0f, 0.0f);
	spotLight->GetDirection().Normalize();


	m_lights.push_back(directionalLight);
	m_lights.push_back(pointLight);
	m_lights.push_back(spotLight);


	m_targetModel = floor;
	m_targetLight = spotLight;

	m_commandLists.resize(1);

	return true;
}

void Engine::Update(float dt)
{
	m_mainCamera->Update(dt);
	for (auto& light : m_lights)
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
