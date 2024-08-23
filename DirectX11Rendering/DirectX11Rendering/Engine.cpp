#include "pch.h"
#include "Engine.h"

bool g_bUsePerspectiveProjection = true;
bool g_bUseDrawNormals = false;
bool g_bUseDrawWireFrame = false;
bool g_bUseDrawBoundingArea = false;
bool g_bUseDrawCameraFrustom = false;

Engine::Engine()
{

}

Engine::~Engine()
{
	delete g_ThreadManager;
}

bool Engine::Initialize()
{
	if(!EngineBase::Initialize())
		return false;

	{
		auto camera = MakeShared<Camera>();
		camera->Initialize(m_device, m_context, GetAspectRatio());
		camera->GetTranslation() = Vector3(0.0f, 0.0f, -30.0);
		camera->GetRotation() = Vector3(0.0f, 0.0f, 0.0f);

		m_cameras.push_back(camera);

		auto otherCamera = MakeShared<Camera>();
		otherCamera->Initialize(m_device, m_context, GetAspectRatio());
		otherCamera->GetTranslation() = Vector3(0.0f, 10.0f, -10.0);
		otherCamera->GetRotation() = Vector3(0.0f, 0.0f, 0.0f);

		m_cameras.push_back(otherCamera);
	}
	
	m_cubeMap = MakeShared<CubeMap>();
	m_cubeMap->Initialize(m_device, m_context, L"../Resources/CubeMaps/skybox/cubemap_bgra.dds", L"../Resources/CubeMaps/skybox/cubemap_diffuse.dds", L"../Resources/CubeMaps/skybox/cubemap_specular.dds");
	m_cubeMap->GetScaling() = Vector3(5.0f, 5.0f, 5.0f);


	{
		m_floor = MakeShared<Model>();
		MeshData gridMeshData{};
		GeometryGenerator::MakeSquareGrid("FloorGrid", 10, 10, gridMeshData);
		m_floor->Initialize(m_device, m_context, Vector<MeshData>{gridMeshData}, true, false);
		m_floor->GetTranslation() = Vector3(0.0f, -0.5f, 0.0f);
		m_floor->GetRotation() = Vector3(3.14/2, 0.0f, 0.0f);
		m_floor->GetScaling() = Vector3(30.0f, 30.0f, 1.0f);
	}

	SetLights();


	auto saveImageFilter = MakeShared<ImageFilter>();
	saveImageFilter->Initialize(m_device, m_context, L"Copy", L"Copy", (float)m_screenWidth, (float)m_screenHeight);
	saveImageFilter->GetIsNotUseThreshold() = true;
	saveImageFilter->SetShaderResources({ m_shaderResourceView });
	saveImageFilter->Update(m_device, m_context, 0);
	m_imageFilters.push_back(saveImageFilter);

	auto copyImageFilter = MakeShared<ImageFilter>();
	copyImageFilter->Initialize(m_device, m_context, L"Copy", L"Copy", (float)m_screenWidth, (float)m_screenHeight);
	copyImageFilter->SetShaderResources({ m_shaderResourceView });
	m_imageFilters.push_back(copyImageFilter);

	for (int i = 0; i < 10; ++i)
	{
		auto blurXImgFilter = MakeShared<ImageFilter>();
		blurXImgFilter->Initialize(m_device, m_context, L"Blur", L"BlurX", m_screenWidth / 4.0f, m_screenHeight / 4.0f);
		blurXImgFilter->SetShaderResources({ m_imageFilters.back()->GetShaderResourceView() });

		m_imageFilters.push_back(blurXImgFilter);

		auto blurYImgFilter = MakeShared<ImageFilter>();
		blurYImgFilter->Initialize(m_device, m_context, L"Blur", L"BlurY", m_screenWidth / 4.0f, m_screenHeight / 4.0f);
		blurYImgFilter->SetShaderResources({ m_imageFilters.back()->GetShaderResourceView() });

		m_imageFilters.push_back(blurYImgFilter);
	}

	auto mergeImageFilter = MakeShared<ImageFilter>();
	mergeImageFilter->Initialize(m_device, m_context, L"Merge", L"Merge", (float)m_screenWidth, (float)m_screenHeight);
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


	for (auto& camera : m_cameras)
	{
		camera->Update(dt);
		camera->UpdateConstantBuffers(m_device, m_context);
	}

	m_directionalLight->Update(dt);

	for (auto& light : m_pointLights)
		light->Update(dt);

	for (auto& light : m_spotLights)
		light->Update(dt);

	UpdateWithThread(dt);

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
	ImGui::Checkbox("Draw BoundingArea", &g_bUseDrawBoundingArea);
	ImGui::Checkbox("Draw Camera Frustom", &g_bUseDrawCameraFrustom);
	
	ImGui::SliderFloat("Blur Threshold", &m_threshold, 0.0f, 0.2f);
	ImGui::SliderFloat("Blur Strength", &m_strength, 0.0f, 0.8f);
	ImGui::SliderFloat("Lod Weight", &m_lodWeight, 1.0f, 64.0f);
	ImGui::SliderFloat("Proxy Dist", &m_proxyMeshAppliedDist, 1.0f, 100.0f);
}

void Engine::Render()
{
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f, 0);

	for (auto& camera : m_cameras)
	{
		camera->Render(m_context);
	}


	RenderWithThread();

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

void Engine::KeyBeginPress(WPARAM wParam)
{

}

void Engine::KeyEndPress(WPARAM wParam)
{
	switch (wParam)
	{
		// c키
	case 0x43:
		m_mainCameraIdx = (m_mainCameraIdx + 1) % m_cameras.size();
		break;
		// I키
	case 0x49:
	{
		if (m_models.size() < MAX_MODEL_ROW_NUM * MAX_MODEL_COL_NUM)
		{
			m_modelLock.WriteLock();
			int idx = m_models.size();
			m_models.push_back(MakeCorset());
			m_models.back()->GetTranslation() = Vector3(-20 + 2 * (idx % MAX_MODEL_COL_NUM), 0.0f, -20 + 2 * (idx / MAX_MODEL_COL_NUM));
			m_modelLock.WriteUnlock();
		}
	}
	break;
		// 'O'키
	case 0x4F:
	{
		if (m_models.size() < MAX_MODEL_ROW_NUM * MAX_MODEL_COL_NUM)
		{
			m_modelLock.WriteLock();
			int idx = m_models.size();
			m_models.push_back(MakeMonster());
			m_models.back()->GetTranslation() = Vector3(-20 + 2 * (idx % MAX_MODEL_COL_NUM), 0.0f, -20 + 2 * (idx / MAX_MODEL_COL_NUM));
			m_modelLock.WriteUnlock();
		}
	}
	break;
		// 'P'키
	case 0x50:
	{
		if (m_models.size() < MAX_MODEL_ROW_NUM * MAX_MODEL_COL_NUM)
		{
			m_modelLock.WriteLock();
			int idx = m_models.size();
			m_models.push_back(MakeDuck());
			m_models.back()->GetTranslation() = Vector3(-20 + 2 * (idx % MAX_MODEL_COL_NUM), 0.0f, -20 + 2 * (idx / MAX_MODEL_COL_NUM));
			m_modelLock.WriteUnlock();
		}
	}
	break;
		// '['키
	case 0xDB:
	{
		if (m_models.size() < MAX_MODEL_ROW_NUM * MAX_MODEL_COL_NUM)
		{
			m_modelLock.WriteLock();
			int idx = m_models.size();
			m_models.push_back(MakeZelda());
			m_models.back()->GetTranslation() = Vector3(-20 + 2 * (idx % MAX_MODEL_COL_NUM), 0.0f, -20 + 2 * (idx / MAX_MODEL_COL_NUM));
			m_modelLock.WriteUnlock();
		}
	}
		break;

		// ']'키
	case 0xDD:
	{
		if (!m_models.empty())
		{
			m_modelLock.WriteLock();
			m_models.pop_back();
			m_modelLock.WriteUnlock();
		}
	}
		break;
	}
}

void Engine::LoadResources()
{



}

void Engine::SetLights()
{
	LightData lightData{};
	m_directionalLight = MakeShared<Light>(lightData);
	m_directionalLight->GetDirection() = Vector3(-1.0f, -1.0f, -1.0);
	m_directionalLight->GetDirection().Normalize();
	m_directionalLight->GetLightData().strength = Vector3(0.5f);

	for (int32 i = 0; i < MAX_LIGHTS; ++i)
	{
		LightData pointLightData{};
		m_pointLights.push_back(MakeShared<Light>(pointLightData));
	}

	for (int32 i = 0; i < MAX_LIGHTS; ++i)
	{
		LightData spotLightData{};
		m_spotLights.push_back(MakeShared<Light>(spotLightData));
	}

	for (int32 i = 0; i < MAX_LIGHTS; ++i)
	{
		m_spotLights[i]->GetLightData().strength = Vector3(1.0f);
		m_spotLights[i]->GetTranslation() = Vector3(-15.0f + 10*(i%4), 3.5f, -15.0f + 10*(i/4));
		m_spotLights[i]->GetDirection() = Vector3(0.0f, -1.0f, 0.0f);
		m_spotLights[i]->GetLightData().fallOffStart = 3.0f;
		m_spotLights[i]->GetLightData().fallOffEnd = 8.0f;
	}
}

void Engine::UpdateWithThread(float dt)
{
	// Model
	uint32 step = ((uint32)m_models.size() + g_ThreadManager->GetMaxThreadCount()) / g_ThreadManager->GetMaxThreadCount();
	for (uint32 i = 0; i < m_models.size(); i += step, 0)
	{
		uint32 startIdx = i;
		uint32 endIdx = std::min(i + step, (uint32)m_models.size());
		auto boundFunc = std::bind(&Engine::UpdateMeshesThread, this, std::placeholders::_1, std::placeholders::_2);
		g_ThreadManager->Launch(boundFunc, ThreadParam{ 0, 0, dt,startIdx, endIdx });
	}

	// Floor
	{
		auto boundFunc = std::bind(&Engine::UpdateFloorThread, this, std::placeholders::_1, std::placeholders::_2);
		g_ThreadManager->Launch(boundFunc, ThreadParam{ 0, 0, dt });
	}

	// CubeMap
	{
		auto boundFunc = std::bind(&Engine::UpdateCubeMapThread, this, std::placeholders::_1, std::placeholders::_2);
		g_ThreadManager->Launch(boundFunc, ThreadParam{ 0, 0, dt });
	}

	g_ThreadManager->Join();
}

bool Engine::UpdateMeshesThread(ThreadParam param, promise<bool>&& pm)
{
	for (uint32 i = param.startIdx; i < param.endIdx; ++i)
	{
		m_models[i]->Update(param.deltatime);
		m_models[i]->UpdateConstantBuffers(m_device, m_context);
	}

	pm.set_value(true);
	return true;
}

bool Engine::UpdateCubeMapThread(ThreadParam param, promise<bool>&& pm)
{
	LthreadID = param.threadID;
	m_cubeMap->Update(param.deltatime);
	m_cubeMap->UpdateConstantBuffers(m_device, m_context);

	pm.set_value(true);
	return true;
}

bool Engine::UpdateFloorThread(ThreadParam param, promise<bool>&& pm)
{
	LthreadID = param.threadID;
	m_floor->Update(param.deltatime);
	m_floor->UpdateConstantBuffers(m_device, m_context);

	pm.set_value(true);
	return true;
}

void Engine::RenderWithThread()
{
	uint32 threadId = 1;
	uint32 step = ((uint32)m_models.size() + g_ThreadManager->GetMaxThreadCount()) / g_ThreadManager->GetMaxThreadCount();
	for (uint32 i = 0; i < m_models.size(); i += step, threadId++)
	{
		uint32 startIdx = i;
		uint32 endIdx = std::min(i + step, (uint32)m_models.size());

		auto boundFunc = std::bind(&Engine::RenderMeshesThread, this, std::placeholders::_1, std::placeholders::_2);
		g_ThreadManager->Launch(boundFunc, ThreadParam{ threadId, threadId, 0,startIdx, endIdx });
	}

	{
		auto boundFunc = std::bind(&Engine::RenderFloorThread, this, std::placeholders::_1, std::placeholders::_2);
		g_ThreadManager->Launch(boundFunc, ThreadParam{ threadId, threadId, 0,0, 0 });
		threadId++;
	}

	{
		auto boundFunc = std::bind(&Engine::RenderCubMapThread, this, std::placeholders::_1, std::placeholders::_2);
		g_ThreadManager->Launch(boundFunc, ThreadParam{ threadId, threadId, 0,0, 0 });
		threadId++;
	}

	g_ThreadManager->Join();

	for (auto& commandList : m_commandLists) {
		if (commandList == nullptr)
			continue;
		m_context->ExecuteCommandList(commandList.Get(), FALSE);
	}

	m_commandLists.clear();

}

bool Engine::RenderMeshesThread(ThreadParam param, promise<bool>&& pm)
{
	LthreadID = param.threadID;
	ComPtr<ID3D11DeviceContext> deferredContext;

	m_deviceLock.ReadLock();
	HRESULT hr =  m_device->CreateDeferredContext(0, &deferredContext);
	if (FAILED(hr))
	{
		std::cerr << "Error: " << "Failed to create deferred context" << " (HRESULT: " << std::hex << hr << ")" << std::endl;
	}
	
	m_deviceLock.ReadUnLock();

	deferredContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get());
	deferredContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(m_screenWidth);
	viewport.Height = static_cast<float>(m_screenHeight); 
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

	pm.set_value(true);
	return true;
}

bool Engine::RenderCubMapThread(ThreadParam param, promise<bool>&& pm)
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

	pm.set_value(true);
	return true;
}

bool Engine::RenderFloorThread(ThreadParam param, promise<bool>&& pm)
{
	LthreadID = param.threadID;
	ComPtr<ID3D11DeviceContext> deferredContext;

	m_deviceLock.ReadLock();
	HRESULT hr = m_device->CreateDeferredContext(0, &deferredContext);
	if (FAILED(hr))
	{
		std::cerr << "Error: " << "Failed to create deferred context" << " (HRESULT: " << std::hex << hr << ")" << std::endl;
	}

	m_deviceLock.ReadUnLock();

	deferredContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get());
	deferredContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(m_screenWidth);
	viewport.Height = static_cast<float>(m_screenHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	deferredContext->RSSetViewports(1, &viewport);

	m_floor->Render(deferredContext);

	ComPtr<ID3D11CommandList> commandList;
	deferredContext->FinishCommandList(FALSE, &commandList);

	m_commandListLock.WriteLock();
	if (m_commandLists.size() <= param.commandListIdx)
		m_commandLists.resize(param.commandListIdx + 1);

	m_commandLists[param.commandListIdx] = commandList;
	m_commandListLock.WriteUnlock();

	pm.set_value(true);
	return true;
}

shared_ptr<ProxyModel> Engine::MakeZelda()
{
	auto zelda = MakeShared<ProxyModel>();
	zelda->Initialize(m_device, m_context, "../Resources/zelda/", "zeldaPosed001.fbx", false);
	zelda->ProxyModeInitialize(m_device, m_context, "../Resources/zelda/", "zeldaPosed001.fbx");
	zelda->GetTranslation() = Vector3(0.0f, 0.0f,0.0f);
	zelda->SetDiffuseResView(m_cubeMap->GetDiffuseResView());
	zelda->SetSpecularResView(m_cubeMap->GetSpecularResView());
	
	return zelda;
}

shared_ptr<ProxyModel> Engine::MakeDuck()
{
	auto duck = MakeShared<ProxyModel>();
	duck->Initialize(m_device, m_context, "../Resources/duck/", "Duck.dae", false);
	duck->ProxyModeInitialize(m_device, m_context, "../Resources/duck/", "Duck.dae");
	duck->GetTranslation() = Vector3(0.0f, 0.0f, 0.0f);
	duck->SetDiffuseResView(m_cubeMap->GetDiffuseResView());
	duck->SetSpecularResView(m_cubeMap->GetSpecularResView());

	return duck;
}

shared_ptr<ProxyModel> Engine::MakeMonster()
{
	auto monster = MakeShared<ProxyModel>();
	monster->Initialize(m_device, m_context, "../Resources/monster/", "Monster.dae", false);
	monster->ProxyModeInitialize(m_device, m_context, "../Resources/monster/", "Monster.dae");
	monster->GetTranslation() = Vector3(0.0f, 0.0f, 0.0f);
	monster->SetDiffuseResView(m_cubeMap->GetDiffuseResView());
	monster->SetSpecularResView(m_cubeMap->GetSpecularResView());

	return monster;
}

shared_ptr<ProxyModel> Engine::MakeCorset()
{
	auto corset = MakeShared<ProxyModel>();
	corset->Initialize(m_device, m_context, "../Resources/corset/", "Corset.fbx", false);
	corset->ProxyModeInitialize(m_device, m_context, "../Resources/corset/", "Corset.fbx");
	corset->GetTranslation() = Vector3(0.0f, 0.0f, 0.0f);
	corset->SetDiffuseResView(m_cubeMap->GetDiffuseResView());
	corset->SetSpecularResView(m_cubeMap->GetSpecularResView());

	return corset;
}
