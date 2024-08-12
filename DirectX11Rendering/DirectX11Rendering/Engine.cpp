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
	m_mainCamera->GetTranslation() = Vector3(0.0f, 0.0f, 10.0f);

	auto floor = std::make_shared<Square>();
	floor->GetTranslation() = Vector3(0.0f, -5.0f, 8.0f);
	floor->GetScaling() = Vector3(10.0f, 10.0f, 1.0f);
	floor->GetRotation() = Vector3(3.14f/2, 0.0f, 0.0f);
	floor->GetMaterial().diffuse = Vector3(1.0f, 1.0f, 1.0f);
	floor->GetMaterial().specular = Vector3(1.0f, 1.0f, 1.0f);

	m_meshes.push_back(floor);

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

	for (auto& mesh : m_meshes)
	{
		mesh->Initialize();
	}

	m_targetMesh = floor;
	m_targetLight = spotLight;

	return true;
}

void Engine::Update(float dt)
{
	m_mainCamera->Update(dt);
	for (auto& light : m_lights)
	{
		light->Update(dt);
	}

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

	if (!m_targetMesh.expired())
	{
		std::shared_ptr<Mesh> targetMesh = m_targetMesh.lock();
		ImGui::SliderFloat3("TargetMesh Translation", &targetMesh->GetTranslation().x, -10.0f, 10.0f);
		ImGui::SliderFloat3("TargetMesh Rotation", &targetMesh->GetRotation().x, -3.14f, 3.14f);
		ImGui::SliderFloat3("TargetMesh Scaling", &targetMesh->GetScaling().x, 0.1f, 10.0f);

		ImGui::SliderFloat3("TargetMesh Material Ambient", &targetMesh->GetMaterial().ambient.x, 0.0f, 1.0f);
		ImGui::SliderFloat3("TargetMesh Material Diffuse", &targetMesh->GetMaterial().diffuse.x, 0.0f, 1.0f);
		ImGui::SliderFloat3("TargetMesh Material Specular", &targetMesh->GetMaterial().specular.x, 0.0f, 1.0f);
		ImGui::SliderFloat("TargetMesh Material Shininess", &targetMesh->GetMaterial().shininess, 0.1f, 2.0f);
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
	m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get());
	m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	for (auto& mesh : m_meshes)
	{
		mesh->Render();
	}
}