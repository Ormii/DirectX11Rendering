#include "pch.h"
#include "EngineUtility.h"

#include <directxtk\DDSTextureLoader.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

void EngineUtility::CreateVertexShaderAndInputLayout(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const wstring& filename, const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements, ComPtr<ID3D11VertexShader>& vertexShader, ComPtr<ID3D11InputLayout>& inputLayout)
{
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif


    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "vs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    CheckResult(hr, errorBlob.Get());

    device->CreateVertexShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), NULL,
        &vertexShader);

    device->CreateInputLayout(inputElements.data(),
        UINT(inputElements.size()),
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), &inputLayout);
}

void EngineUtility::CreatePixelShader(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const wstring& filename, ComPtr<ID3D11PixelShader>& pixelShader)
{
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;


    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "ps_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    CheckResult(hr, errorBlob.Get());

    device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &pixelShader);
}

void EngineUtility::CreateIndexBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const vector<uint32>& indices, ComPtr<ID3D11Buffer>& m_indexBuffer)
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

    device->CreateBuffer(&bufferDesc, &indexBufferData, &m_indexBuffer);
}

void EngineUtility::CreateTexture(ComPtr<ID3D11Device>&device, const std::string filename,
    ComPtr<ID3D11Texture2D>& texture,
    ComPtr<ID3D11ShaderResourceView>& textureResourceView)
{
    int width, height, channels;

    unsigned char* img =
        stbi_load(filename.c_str(), &width, &height, &channels, 0);

    std::vector<uint8_t> image(width * height * 4, 0);
    for (size_t i = 0; i < width * height; ++i)
    {
        for (size_t c = 0; c < 3; ++c)
            image[4 * i + c] = img[i * channels + c];
        image[4 * i + 3] = 255;
    }

    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.MipLevels = textureDesc.ArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData{};
    initData.pSysMem = image.data();
    initData.SysMemPitch = textureDesc.Width * sizeof(uint8_t) * 4;

    device->CreateTexture2D(&textureDesc, &initData, texture.GetAddressOf());
    device->CreateShaderResourceView(texture.Get(), nullptr, textureResourceView.GetAddressOf());
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

