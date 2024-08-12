#include "pch.h"
#include "Engine.h"

bool g_bUsePerspectiveProjection = true;
bool g_bUseDrawNormals = true;
bool g_bUseDrawWireFrame = false;

Engine::Engine()
{

}

Engine::~Engine()
{

}

bool Engine::Initialize()
{
	if(!EngineBase::Initialize())
		return false;

	m_mainCamera = std::make_shared<Camera>();

	auto floor = std::make_shared<Square>();
	floor->GetRotation() = Vector3(-90.0f, 0.0f, 0.0f);

	m_meshes.push_back(floor);
	

	for (auto& mesh : m_meshes)
	{
		mesh->Initialize();
	}

	return true;
}

void Engine::Update(float dt)
{
	m_mainCamera->Update();
	for (auto& mesh : m_meshes)
	{
		mesh->Update(dt);
	}

}

void Engine::UpdateGUI()
{
	ImGui::Checkbox("Draw WireFrame", &g_bUseDrawWireFrame);
	ImGui::Checkbox("Draw Normals", &g_bUseDrawNormals);
	ImGui::Checkbox("Use Perspective", &g_bUsePerspectiveProjection);
}

void Engine::Render()
{
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f, 0);
	m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get());
	m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	for (auto& mesh : m_meshes)
	{
		mesh->Render();
	}

}
