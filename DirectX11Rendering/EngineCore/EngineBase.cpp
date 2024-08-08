#include "pch.h"
#include "EngineBase.h"

EngineBase* g_EngineBase = nullptr;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	return g_EngineBase->MsgProc(hWnd, msg, wParam, lParam);
}

EngineBase::EngineBase()
	:m_screenWidth(MAIN_WINDOW_WIDTH), m_screenHeight(MAIN_WINDOW_HEIGHT), m_hwnd(0), m_viewport(D3D11_VIEWPORT())
{
	g_EngineBase = this;
}

EngineBase::~EngineBase()
{
    DestroyWindow(m_hwnd);
	
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
            Update();
            Render();
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

    return true;
}

LRESULT EngineBase::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
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

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_9_3 };

    D3D_FEATURE_LEVEL featureLevel;

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
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;

    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr,
        driverType,
        0,
        createDeviceFlags,
        featureLevels,
        1,
        D3D11_SDK_VERSION,
        &sd,
        &m_swapChain,
        &m_device,
        &featureLevel,
        &m_context
    )))
    {
        return false;
    }

    if (featureLevel != D3D_FEATURE_LEVEL_11_0)
    {
        return false;
    }

    UINT numQualityLevels = 0;
    m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &numQualityLevels);
  

//-----------------------------------------------------------------------
//                      RenderTargetView  
//-----------------------------------------------------------------------

    ID3D11Texture2D* pBackBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer)
    {
        m_device->CreateRenderTargetView(pBackBuffer, NULL, &m_renderTargetView);
        pBackBuffer->Release();
    }
    else 
    {
        return false;
    }

/*-----------------------------------------------------------------------
*                           Rasterizer
-------------------------------------------------------------------------*/

    D3D11_RASTERIZER_DESC rastDesc;
    ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    rastDesc.FrontCounterClockwise = false;

    m_device->CreateRasterizerState(&rastDesc, &m_rasterizerState);

/*-----------------------------------------------------------------------
*                           View port
-------------------------------------------------------------------------*/

    ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_viewport.Width = float(m_screenWidth);
    m_viewport.Height = float(m_screenHeight);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &m_viewport);


/*-----------------------------------------------------------------------
*                           Depth Stencil
-------------------------------------------------------------------------*/

    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    depthStencilBufferDesc.Width = m_screenWidth;
    depthStencilBufferDesc.Height = m_screenHeight;
    depthStencilBufferDesc.MipLevels = 1;
    depthStencilBufferDesc.ArraySize = 1;
    depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    if (numQualityLevels > 0) {
        depthStencilBufferDesc.SampleDesc.Count = 4;
        depthStencilBufferDesc.SampleDesc.Quality = numQualityLevels - 1;
    }
    else {
        depthStencilBufferDesc.SampleDesc.Count = 1;
        depthStencilBufferDesc.SampleDesc.Quality = 0;
    }
    depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilBufferDesc.CPUAccessFlags = 0;
    depthStencilBufferDesc.MiscFlags = 0;

    if (FAILED(m_device->CreateTexture2D(&depthStencilBufferDesc, 0,
        m_depthStencilBuffer.GetAddressOf())))
    {    
        return false;
    }
    if (FAILED(
        m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), 0, &m_depthStencilView))) 
    {
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
    if (FAILED(m_device->CreateDepthStencilState(&depthStencilDesc,
        m_depthStencilState.GetAddressOf()))) 
    {
        return false;
    }

    return true;
}

void EngineBase::CreateVertexShaderAndInputLayout(const wstring& filename, const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements, ComPtr<ID3D11VertexShader>& vertexShader, ComPtr<ID3D11InputLayout>& inputLayout)
{
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompileFromFile(filename.c_str(), 0, 0, "main", "vs_5_0", 0, 0, &shaderBlob, &errorBlob);
        
    m_device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &vertexShader);
    m_device->CreateInputLayout(inputElements.data(), UINT(inputElements.size()), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &inputLayout);
}

void EngineBase::CreatePixelShader(const wstring& filename, ComPtr<ID3D11PixelShader>& pixelShader)
{
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompileFromFile(filename.c_str(), 0, 0, "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob);

    m_device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &pixelShader);
}

void EngineBase::CreateIndexBuffer(const vector<uint16_t>& indices, ComPtr<ID3D11Buffer>& m_indexBuffer)
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.ByteWidth = UINT(sizeof(uint16_t) * indices.size());
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.StructureByteStride = sizeof(uint16_t);

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0, };
    indexBufferData.pSysMem = indices.data();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    
    m_device->CreateBuffer(&bufferDesc, &indexBufferData, &m_indexBuffer);
}
