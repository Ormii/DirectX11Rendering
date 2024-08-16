#include "pch.h"
#include "EngineBase.h"
#include "ThreadManager.h"

EngineBase* g_EngineBase = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	return g_EngineBase->MsgProc(hWnd, msg, wParam, lParam);
}

EngineBase::EngineBase()
	:m_screenWidth(MAIN_WINDOW_WIDTH), m_screenHeight(MAIN_WINDOW_HEIGHT), m_hwnd(0), m_numQualityLevels(0), m_viewport(D3D11_VIEWPORT())
{
	g_EngineBase = this;
}

EngineBase::~EngineBase()
{
    DestroyWindow(m_hwnd);

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    g_EngineBase = nullptr;
}

int EngineBase::Run()
{
    MSG msg = { 0 };
    while (WM_QUIT != msg.message) 
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else 
        {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();

            ImGui::NewFrame();
            ImGui::Begin("Control");

            ImGui::Text("%.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

            ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));

            UpdateGUI();

            ImGui::End();
            ImGui::Render();

            Update(ImGui::GetIO().DeltaTime);
            Render();

            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            m_swapChain->Present(1, 0);
        }
    }

    return 0;
}

bool EngineBase::Initialize()
{
    if (!InitMainWindow())
        return false;

    if (!InitDirectX())
        return false;

    if (!InitGUI())
        return false;

    return true;
}

LRESULT EngineBase::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        case WM_MOUSEMOVE:
        {
            m_mouseCursorX = LOWORD(lParam);
            m_mouseCursorY = HIWORD(lParam);

            OnMouseMove(wParam, m_mouseCursorX, m_mouseCursorY);
        }
            break;
        case WM_LBUTTONDOWN:
        {
            m_leftButtonDown = true;
        }
            break;
        case WM_LBUTTONUP:
        {
            m_leftButtonDown = false;
        }
            break;
        case WM_RBUTTONDOWN:
        {
            m_rightButtonDowm = true;
        }
            break;
        case WM_RBUTTONUP:
        {
            m_rightButtonDowm = false;
        }
            break;
        case WM_KEYDOWN:
        {
            m_keyPressed[wParam] = true;
        }
            break;
        case WM_KEYUP:
        {
            m_keyPressed[wParam] = false;
        }
            break;
        case WM_DESTROY:
        {
            ::PostQuitMessage(0);
            return 0;
        }
            break;
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

bool EngineBase::InitMainWindow()
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX),
                     CS_CLASSDC,
                     WndProc,
                     0L,
                     0L,
                     GetModuleHandle(NULL),
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     L"Engine",
                     NULL };

    if (!RegisterClassEx(&wc)) 
    {
        return false;
    }

    RECT wr = { 0, 0, m_screenWidth, m_screenHeight };
    DWORD dwStyle = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    AdjustWindowRect(&wr, dwStyle, false);

    m_hwnd = CreateWindow(wc.lpszClassName, L"Engine", dwStyle,
        300,                
        10,                
        wr.right - wr.left,
        wr.bottom - wr.top,
        NULL, NULL, wc.hInstance, NULL);

    if (!m_hwnd) 
    {
        return false;
    }

    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hwnd);

    return true;
}

bool EngineBase::InitDirectX()
{
//-----------------------------------------------------------------------
//                  Device, Context, SwapChain       
//-----------------------------------------------------------------------

    const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_9_3 };

    D3D_FEATURE_LEVEL featureLevel;

    if (FAILED(D3D11CreateDevice(
        nullptr,    
        driverType,
        0, 
        createDeviceFlags, 
        featureLevels,     
        ARRAYSIZE(featureLevels), 
        D3D11_SDK_VERSION,  
      
        device.GetAddressOf(), 
        &featureLevel,        
        context.GetAddressOf() 
    ))) {
       
        return false;
    }

    if (featureLevel != D3D_FEATURE_LEVEL_11_0)
    {
        return false;
    }

    device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_numQualityLevels);

    if (m_numQualityLevels <= 0) {
        return false;
    }

    if (FAILED(device.As(&m_device))) {
        return false;
    }

    if (FAILED(context.As(&m_context))) {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = m_screenWidth;
    sd.BufferDesc.Height = m_screenHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.Windowed = TRUE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    if (m_numQualityLevels > 0) {
        sd.SampleDesc.Count = 4;
        sd.SampleDesc.Quality = m_numQualityLevels - 1;
    }
    else {
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
    }

    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr,
        driverType,
        0,
        createDeviceFlags,
        featureLevels,
        1,
        D3D11_SDK_VERSION,
        &sd,
        m_swapChain.GetAddressOf(),
        m_device.GetAddressOf(),
        &featureLevel,
        m_context.GetAddressOf()
    )))
    {
        return false;
    }

    if (!CreateRenderTargetView())
        return false;

    SetViewport();

    if (!CreateRasterizerState())
        return false;


    if (!EngineUtility::CreateDepthStencilBuffer(m_device,m_context,m_depthStencilBuffer,m_depthStencilView,
        m_depthStencilState,m_screenWidth,m_screenHeight, m_numQualityLevels))
        return false;

    return true;
}

bool EngineBase::InitGUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.DisplaySize = ImVec2(float(m_screenWidth), float(m_screenHeight));
    ImGui::StyleColorsDark();

    if (!ImGui_ImplDX11_Init(m_device.Get(), m_context.Get())) {
        return false;
    }

    if (!ImGui_ImplWin32_Init(m_hwnd)) {
        return false;
    }

    return true;
}

void EngineBase::OnMouseMove(WPARAM wParam, int mouseX, int mouseY)
{
    float aspect = GetAspectRatio();
    m_mouseCursorNdcX = static_cast<float>(mouseX * 2.0f )/ m_screenWidth - 1.0f;
    m_mouseCursorNdxY = -static_cast<float>(mouseY * 2.0f)/ m_screenHeight + 1.0f;

    m_mouseCursorNdcX = std::clamp(m_mouseCursorNdcX, -1.0f, 1.0f);
    m_mouseCursorNdxY = std::clamp(m_mouseCursorNdxY, -1.0f, 1.0f);
}

float EngineBase::GetAspectRatio()
{
    return float(m_screenWidth) / m_screenHeight;
}

bool EngineBase::CreateRenderTargetView()
{
    ComPtr<ID3D11Texture2D> pBackBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
    if (pBackBuffer)
    {
        m_device->CreateRenderTargetView(pBackBuffer.Get(), NULL, m_renderTargetView.GetAddressOf());
    }
    else
    {
        return false;
    }

    return true;
}

bool EngineBase::CreateRasterizerState()
{
    D3D11_RASTERIZER_DESC solidRastDesc;
    ZeroMemory(&solidRastDesc, sizeof(D3D11_RASTERIZER_DESC));
    solidRastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    solidRastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    solidRastDesc.FrontCounterClockwise = false;

    if (FAILED(m_device->CreateRasterizerState(&solidRastDesc, &m_solidRasterizerState)))
        return false;


    D3D11_RASTERIZER_DESC wireRastDesc;
    ZeroMemory(&wireRastDesc, sizeof(D3D11_RASTERIZER_DESC));
    wireRastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
    wireRastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    wireRastDesc.FrontCounterClockwise = false;

    if (FAILED(m_device->CreateRasterizerState(&wireRastDesc, &m_wireRasterizerState)))
        return false;

    return true;
}

void EngineBase::SetViewport()
{
    ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_viewport.Width = float(m_screenWidth);
    m_viewport.Height = float(m_screenHeight);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &m_viewport);
}
