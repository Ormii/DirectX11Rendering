#include "pch.h"
#include "EngineUtility.h"

#include <directxtk\DDSTextureLoader.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


mutex EngineUtility::lock;

bool EngineUtility::CreateDepthStencilBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
    ComPtr<ID3D11Texture2D>& depthStencilBuffer, ComPtr<ID3D11DepthStencilView>& depthStencilView,
    ComPtr<ID3D11DepthStencilState>& depthStencilState, int32 screenWidth, int32 screenHeight, UINT numQualityLevels)
{
    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    depthStencilBufferDesc.Width = screenWidth;
    depthStencilBufferDesc.Height = screenHeight;
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

    if (FAILED(device->CreateTexture2D(&depthStencilBufferDesc, 0,
        depthStencilBuffer.GetAddressOf())))
    {
        return false;
    }
    if (FAILED(
        device->CreateDepthStencilView(depthStencilBuffer.Get(), 0, depthStencilView.GetAddressOf())))
    {
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
    if (FAILED(device->CreateDepthStencilState(&depthStencilDesc,
        depthStencilState.GetAddressOf())))
    {
        return false;
    }

    return true;
}

void CheckResult(HRESULT hr, ID3DBlob* errorBlob) {
    if (FAILED(hr)) {

        if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
            std::cout << "File not found." << std::endl;
        }

        if (errorBlob) {
            std::cout << "Shader compile error\n"
                << (char*)errorBlob->GetBufferPointer() << std::endl;
        }
    }
}

void EngineUtility::CreateVertexShaderAndInputLayout(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const WString& filename, const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements, ComPtr<ID3D11VertexShader>& vertexShader, ComPtr<ID3D11InputLayout>& inputLayout)
{
    ComPtr<ID3DBlob> errorBlob;

    VertexShaderInfo vsInfo{};


    g_ResourceManager->m_vertexShaderLock.WriteLock();
    if (!g_ResourceManager->GetVsShader(filename, vsInfo))
    {
        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif


        HRESULT hr = D3DCompileFromFile(
            filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
            "vs_5_0", compileFlags, 0, &vsInfo.m_shaderBlob, &errorBlob);

        CheckResult(hr, errorBlob.Get());

        device->CreateVertexShader(vsInfo.m_shaderBlob->GetBufferPointer(),
            vsInfo.m_shaderBlob->GetBufferSize(), NULL,
            &vsInfo.m_vertexShader);

        device->CreateInputLayout(inputElements.data(),
            UINT(inputElements.size()),
            vsInfo.m_shaderBlob->GetBufferPointer(),
            vsInfo.m_shaderBlob->GetBufferSize(), &vsInfo.m_inputLayout);

        g_ResourceManager->SetVsShader(filename, vsInfo);
    }
    g_ResourceManager->m_vertexShaderLock.WriteUnlock();

    vertexShader = vsInfo.m_vertexShader;
    inputLayout = vsInfo.m_inputLayout;
}

void EngineUtility::CreatePixelShader(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const WString& filename, ComPtr<ID3D11PixelShader>& pixelShader)
{
    ComPtr<ID3DBlob> errorBlob;

    PixelShaderInfo psInfo{};

    g_ResourceManager->m_pixelShaderLock.WriteLock();
    if (!g_ResourceManager->GetPsShader(filename, psInfo))
    {
        UINT compileFlags = 0;
    #if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    #endif

        HRESULT hr = D3DCompileFromFile(
            filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
            "ps_5_0", compileFlags, 0, &psInfo.m_shaderBlob, &errorBlob);

        CheckResult(hr, errorBlob.Get());

        device->CreatePixelShader(psInfo.m_shaderBlob->GetBufferPointer(), psInfo.m_shaderBlob->GetBufferSize(), NULL, &psInfo.m_pixelShader);

        g_ResourceManager->SetPsShader(filename, psInfo);
    }
    g_ResourceManager->m_pixelShaderLock.WriteUnlock();

    pixelShader = psInfo.m_pixelShader;

}

void EngineUtility::CreateIndexBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const Vector<uint32>& indices, ComPtr<ID3D11Buffer>& indexBuffer)
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.ByteWidth = UINT(sizeof(uint32) * indices.size());
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.StructureByteStride = sizeof(uint32);

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0, };
    indexBufferData.pSysMem = indices.data();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;

    device->CreateBuffer(&bufferDesc, &indexBufferData, &indexBuffer);
}

void EngineUtility::CreateHullShader(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const WString& filename, ComPtr<ID3D11HullShader>& hullShader)
{
    ComPtr<ID3DBlob> errorBlob;

    HullShaderInfo hsInfo{};
    g_ResourceManager->m_hullShaderLock.WriteLock();
    if (!g_ResourceManager->GetHsShader(filename, hsInfo))
    {
        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        // ���̴��� �������� �̸��� "main"�� �Լ��� ����
        // D3D_COMPILE_STANDARD_FILE_INCLUDE �߰�: ���̴����� include ���
        HRESULT hr = D3DCompileFromFile(
            filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
            "hs_5_0", compileFlags, 0, &hsInfo.m_shaderBlob, &errorBlob);

        CheckResult(hr, errorBlob.Get());

        device->CreateHullShader(hsInfo.m_shaderBlob->GetBufferPointer(),
            hsInfo.m_shaderBlob->GetBufferSize(), NULL, &hsInfo.m_hullShader);
    }
    g_ResourceManager->m_hullShaderLock.WriteUnlock();

    hullShader = hsInfo.m_hullShader;
}

void EngineUtility::CreateDomainShader(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const WString& filename, ComPtr<ID3D11DomainShader>& domainShader)
{
    ComPtr<ID3DBlob> errorBlob;
    DomainShaderInfo dsInfo{};
    
    g_ResourceManager->m_domainShaderLock.WriteLock();
    if (!g_ResourceManager->GetDsShader(filename, dsInfo))
    {
        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif


        HRESULT hr = D3DCompileFromFile(
            filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
            "ds_5_0", compileFlags, 0, &dsInfo.m_shaderBlob, &errorBlob);

        CheckResult(hr, errorBlob.Get());

        device->CreateDomainShader(dsInfo.m_shaderBlob->GetBufferPointer(),
            dsInfo.m_shaderBlob->GetBufferSize(), NULL,
            &dsInfo.m_domainShader);
    }
    g_ResourceManager->m_domainShaderLock.WriteUnlock();

    domainShader = dsInfo.m_domainShader;
}

void EngineUtility::CreateTexture(ComPtr<ID3D11Device>&device, const String filename,
    ComPtr<ID3D11Texture2D>& texture,
    ComPtr<ID3D11ShaderResourceView>& textureResourceView)
{
    TextureInfo textureInfo{};

    g_ResourceManager->m_textureLock.WriteLock();
    if (!g_ResourceManager->GetTexture(filename, textureInfo))
    {
        unsigned char* img =
            stbi_load(filename.c_str(), &textureInfo.m_width, &textureInfo.m_height, &textureInfo.m_channels, 0);
        textureInfo.m_image.resize(textureInfo.m_width * textureInfo.m_height * 4);
        for (size_t i = 0; i < textureInfo.m_width * textureInfo.m_height; ++i)
        {
            for (size_t c = 0; c < 3; ++c)
                textureInfo.m_image[4 * i + c] = img[i * textureInfo.m_channels + c];
            textureInfo.m_image[4 * i + 3] = 255;
        }

        D3D11_TEXTURE2D_DESC textureDesc{};
        textureDesc.Width = textureInfo.m_width;
        textureDesc.Height = textureInfo.m_height;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.MipLevels = textureDesc.ArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = textureInfo.m_image.data();
        initData.SysMemPitch = textureDesc.Width * sizeof(uint8_t) * 4;

        device->CreateTexture2D(&textureDesc, &initData, textureInfo.m_texture.GetAddressOf());
        device->CreateShaderResourceView(textureInfo.m_texture.Get(), nullptr, textureInfo.m_shaderResView.GetAddressOf());

        g_ResourceManager->SetTexture(filename, textureInfo);
    }
    g_ResourceManager->m_textureLock.WriteUnlock();

    texture = textureInfo.m_texture;
    textureResourceView = textureInfo.m_shaderResView;
}

void EngineUtility::CreateCubemapTexture(ComPtr<ID3D11Device>& device, const wchar_t* filename, ComPtr<ID3D11ShaderResourceView>& texResView)
{
    ComPtr<ID3D11Texture2D> texture;

    auto hr = CreateDDSTextureFromFileEx(
        device.Get(), filename, 0, D3D11_USAGE_DEFAULT,
        D3D11_BIND_SHADER_RESOURCE, 0,
        D3D11_RESOURCE_MISC_TEXTURECUBE,
        DDS_LOADER_FLAGS(false), (ID3D11Resource**)texture.GetAddressOf(),
        texResView.GetAddressOf(), nullptr);

    if (FAILED(hr)) 
    {
        std::cout << "CreateDDSTextureFromFileEx() failed" << std::endl;
    }
}

