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
	m_mainCamera->GetTranslation() = Vector3(0.0f, 0.0f, -30.0);
	m_mainCamera->GetRotation() = Vector3(0.0f, 3.14f, 0.0f);

	m_cubeMap = make_shared<CubeMap>();
	m_cubeMap->Initialize(m_device, m_context, L"../Resources/CubeMaps/skybox/cubemap_bgra.dds", L"../Resources/CubeMaps/skybox/cubemap_diffuse.dds", L"../Resources/CubeMaps/skybox/cubemap_specular.dds");


	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			auto zelda = make_shared<Model>();
			zelda->Initialize(m_device, m_context, "../Resources/zelda/", "zeldaPosed001.fbx");
			zelda->GetTranslation() = Vector3(-1 + i, 0.0f, -1 + j);
			zelda->GetScaling() = Vector3(4.0f, 4.0f, 4.0f);
			zelda->SetDiffuseResView(m_cubeMap->GetDiffuseResView());
			zelda->SetSpecularResView(m_cubeMap->GetSpecularResView());
			m_models.push_back(zelda);
		}
	}

	auto floor = make_shared<Model>();
	floor->Initialize(m_device, m_context, vector<MeshData>{GeometryGenerator::MakeBox(1.0f)});
	floor->GetTranslation() = Vector3(0.0f, -3.0f, 0.0f);
	floor->GetRotation() = Vector3(3.14f / 2, 0.0f, 0.0f);
	floor->GetScaling() = Vector3(10.0f, 10.0f, 1.0f);
	floor->SetDiffuseResView(m_cubeMap->GetDiffuseResView());
	floor->SetSpecularResView(m_cubeMap->GetSpecularResView());
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

	m_targetModel = m_models.back();
	m_targetLight = m_spotLights[0];


	auto saveImageFilter = make_shared<ImageFilter>();
	saveImageFilter->Initialize(m_device, m_context, L"Copy", L"Copy", m_screenWidth, m_screenHeight);
	saveImageFilter->GetIsNotUseThreshold() = true;
	saveImageFilter->SetShaderResources({ m_shaderResourceView });
	saveImageFilter->Update(m_device, m_context, 0);
	m_imageFilters.push_back(saveImageFilter);

	auto copyImageFilter = make_shared<ImageFilter>();
	copyImageFilter->Initialize(m_device, m_context, L"Copy", L"Copy", m_screenWidth, m_screenHeight);
	copyImageFilter->SetShaderResources({ m_shaderResourceView });
	m_imageFilters.push_back(copyImageFilter);

	for (int i = 0; i < 10; ++i)
	{
		auto blurXImgFilter = make_shared<ImageFilter>();
		blurXImgFilter->Initialize(m_device, m_context, L"Blur", L"BlurX", m_screenWidth / 4, m_screenHeight / 4);
		blurXImgFilter->SetShaderResources({ m_imageFilters.back()->GetShaderResourceView() });

		m_imageFilters.push_back(blurXImgFilter);

		auto blurYImgFilter = make_shared<ImageFilter>();
		blurYImgFilter->Initialize(m_device, m_context, L"Blur", L"BlurY", m_screenWidth / 4, m_screenHeight / 4);
		blurYImgFilter->SetShaderResources({ m_imageFilters.back()->GetShaderResourceView() });

		m_imageFilters.push_back(blurYImgFilter);
	}

	auto mergeImageFilter = make_shared<ImageFilter>();
	mergeImageFilter->Initialize(m_device, m_context, L"Merge", L"Merge", m_screenWidth, m_screenHeight);
	mergeImageFilter->SetShaderResources({ m_imageFilters.front()->GetShaderResourceView(),m_imageFilters.back()->GetShaderResourceView() });
	mergeImageFilter->SetRenderTargets({ m_renderTargetView });

	m_imageFilters.push_back(mergeImageFilter);


	return true;
}

void Engine::Update(float dt)
{

	if (m_rightButtonDowm)
	{
		if (m_keyPressed[87])
			GetMainCamera()->MoveForward(dt);
		if (m_keyPressed[83])
			GetMainCamera()->MoveForward(-dt);
		if (m_keyPressed[68])
			GetMainCamera()->MoveRight(dt);
		if (m_keyPressed[65])
			GetMainCamera()->MoveRight(-dt);
	}

	if (m_keyPressed[0x10])
		GetMainCamera()->SetSpeedUp(true);
	else
		GetMainCamera()->SetSpeedUp(false);


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

	uint32 threadId = 1;
	uint32 step = (m_models.size() + g_ThreadManager->GetMaxThreadCount()) / g_ThreadManager->GetMaxThreadCount();
	for (uint32 i = 0; i < m_models.size(); i += step, threadId++)
	{
		uint32 startIdx = i;
		uint32 endIdx = std::min(i + step,(uint32)m_models.size());
		g_ThreadManager->Launch(&Engine::UpdateMeshes, this, ThreadParam{ threadId, threadId, dt,startIdx, endIdx });
	}

	g_ThreadManager->Launch(&Engine::UpdateCubeMap, this, ThreadParam{ threadId, threadId, dt });

	g_ThreadManager->Join();

	for (auto& imageFilter : m_imageFilters)
	{
		imageFilter->Update(m_device, m_context, dt);

		imageFilter->GetPixelConstantBufferData().strength = m_strength;
		if(imageFilter->GetIsNotUseThreshold() == false)
			imageFilter->GetPixelConstantBufferData().threshold = m_threshold;
	}
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

	ImGui::SliderFloat("Blur Threshold", &m_threshold, 0.0f, 1.0f);
	ImGui::SliderFloat("Blur Strength", &m_strength, 0.0f, 20.0f);
}

void Engine::Render()
{
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f, 0);

	uint32 threadId = 1;
	uint32 step = (m_models.size() + g_ThreadManager->GetMaxThreadCount()) / g_ThreadManager->GetMaxThreadCount();
	for (uint32 i = 0; i < m_models.size(); i += step, threadId++)
	{
		uint32 startIdx = i;
		uint32 endIdx = std::min(i + step, (uint32)m_models.size());
		g_ThreadManager->Launch(&Engine::RenderMeshes, this, ThreadParam{ threadId, threadId, 0,startIdx, endIdx });
	}

	{
		g_ThreadManager->Launch(&Engine::RenderCubMap, this, ThreadParam{ threadId, threadId, 0,0, 0 });
		threadId++;
	}

	g_ThreadManager->Join();

	for (auto& commandList : m_commandLists) {
		if (commandList == nullptr)
			continue;
		m_context->ExecuteCommandList(commandList.Get(), FALSE);
	}

	m_commandLists.clear();

	ComPtr<ID3D11Texture2D> backBuffer;
	m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
	m_context->ResolveSubresource(m_tempTexture.Get(), 0, backBuffer.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	for (auto& imageFilter : m_imageFilters)
	{
		imageFilter->Render(m_context);
	}

	m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get());
	m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
}

void Engine::OnMouseMove(WPARAM wParam, int mouseX, int mouseY)
{
	EngineBase::OnMouseMove(wParam, mouseX, mouseY);

	weak_ptr<Camera> pWeakCamera = GetMainCamera();
	if (pWeakCamera.expired())
		return;

	shared_ptr<Camera> pCamera = pWeakCamera.lock();

	if(m_rightButtonDowm)
		pCamera->UpdateMouse(m_mouseCursorNdcX - m_mousePrevCurserNdcX, m_mouseCursorNdxY - m_mousePrevCurserNdcY);
}

void Engine::LoadResources()
{



}

void Engine::UpdateMeshes(ThreadParam param)
{
	for (int32 i = param.startIdx; i < param.endIdx; ++i)
	{
		m_models[i]->Update(param.deltatime);
		m_models[i]->UpdateConstantBuffers(m_device, m_context);
	}
}

void Engine::UpdateCubeMap(ThreadParam param)
{
	LthreadID = param.threadID;
	m_cubeMap->Update(m_device, m_context, param.deltatime);
}

void Engine::RenderMeshes(ThreadParam param)
{
	LthreadID = param.threadID;
	ComPtr<ID3D11DeviceContext> deferredContext;

	m_deviceLock.ReadLock();
	HRESULT hr =  m_device->CreateDeferredContext(0, &deferredContext);
	if (FAILED(hr)) // HRESULT가 실패 상태인지 확인
	{
		std::cerr << "Error: " << "Failed to create deferred context" << " (HRESULT: " << std::hex << hr << ")" << std::endl;

		// 추가로 필요한 오류 처리 (예: 오류에 따른 리소스 정리)
		// ...

		// 예외를 던지거나 프로그램을 종료할 수도 있습니다.
		// throw std::runtime_error("HRESULT failed");
	}
	
	m_deviceLock.ReadUnLock();

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

	for (int32 i = param.startIdx; i < param.endIdx; ++i)
	{
		m_models[i]->Render(deferredContext);
	}

	ComPtr<ID3D11CommandList> commandList;
	deferredContext->FinishCommandList(FALSE, &commandList);

	m_commandListLock.WriteLock();
	if (m_commandLists.size() <= param.commandListIdx)
		m_commandLists.resize(param.commandListIdx + 1);

	m_commandLists[param.commandListIdx] = commandList;
	m_commandListLock.WriteUnlock();
}

void Engine::RenderCubMap(ThreadParam param)
{
	LthreadID = param.threadID;
	ComPtr<ID3D11DeviceContext> deferredContext;

	m_deviceLock.ReadLock();
	m_device->CreateDeferredContext(0, &deferredContext);
	m_deviceLock.ReadUnLock();

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

	m_cubeMap->Render(m_device, deferredContext);

	ComPtr<ID3D11CommandList> commandList;
	deferredContext->FinishCommandList(FALSE, &commandList);

	m_commandListLock.WriteLock();
	if (m_commandLists.size() <= param.commandListIdx)
		m_commandLists.resize(param.commandListIdx + 1);

	m_commandLists[param.commandListIdx] = commandList;
	m_commandListLock.WriteUnlock();
}
