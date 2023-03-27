#include "post_effect.h"
#include "utils.h"

#include <directxmath.h>
#include <exception>

namespace
{

     struct ConstBuffer
     {
          DirectX::XMINT4 params;
          DirectX::XMFLOAT4 size;
     };

}

PostEffect::PostEffect(ID3D11Device *device, HWND hwnd, const unsigned width, const unsigned height) :
     width_(width),
     height_(height),
     useSobel_(defaultUseSobel_),
     useGray_(defaultUseGray_),
     pVertexShader_(nullptr),
     pPixelShader_(nullptr),
     pSamplerState_(nullptr),
     pConstBuffer_(nullptr)
{
     ID3DBlob *pVertexShaderBlob = NULL;
     auto result = CompileShaderFromFile(L"post_effect_vertex.hlsl", "main", "vs_5_0", &pVertexShaderBlob);
     if (FAILED(result))
          throw std::exception("Failed to compile vertex shader");

     result = device->CreateVertexShader(
          pVertexShaderBlob->GetBufferPointer(),
          pVertexShaderBlob->GetBufferSize(),
          NULL,
          &pVertexShader_);
     pVertexShaderBlob->Release();
     if (FAILED(result))
          throw std::exception("Failed to create vertex shader");

     ID3DBlob *pPixelShaderBlob = NULL;
     result = CompileShaderFromFile(L"post_effect_pixel.hlsl", "main", "ps_5_0", &pPixelShaderBlob);
     if (FAILED(result))
          throw std::exception("Failed to compile pixel shader");

     result = device->CreatePixelShader(
          pPixelShaderBlob->GetBufferPointer(),
          pPixelShaderBlob->GetBufferSize(),
          nullptr,
          &pPixelShader_);
     pPixelShaderBlob->Release();
     if (FAILED(result))
          throw std::exception("Failed to compile pixel shader");

     D3D11_SAMPLER_DESC samplerDesc;
     ZeroMemory(&samplerDesc, sizeof(samplerDesc));
     samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
     samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
     samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
     samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
     samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
     samplerDesc.MinLOD = 0;
     samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
     samplerDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
     result = device->CreateSamplerState(&samplerDesc, &pSamplerState_);
     if (FAILED(result))
          throw std::exception("Failed to create sampler state");

     D3D11_BUFFER_DESC desc = {};
     desc.ByteWidth = sizeof(ConstBuffer);
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     ConstBuffer constBuffer_;
     constBuffer_.params = DirectX::XMINT4(useSobel_, useGray_, 0, 0);
     constBuffer_.size = DirectX::XMFLOAT4(1 / static_cast<float>(width_), 1 / static_cast<float>(height_), 0, 0);

     D3D11_SUBRESOURCE_DATA data;
     data.pSysMem = &constBuffer_;
     data.SysMemPitch = sizeof(constBuffer_);
     data.SysMemSlicePitch = 0;

     result = device->CreateBuffer(&desc, &data, &pConstBuffer_);
     if (FAILED(result))
          throw std::exception("Failed to create constant buffer");
}

PostEffect::~PostEffect()
{
     SafeRelease(pVertexShader_);
     SafeRelease(pPixelShader_);
     SafeRelease(pSamplerState_);
     SafeRelease(pConstBuffer_);
}

void PostEffect::Resize(const unsigned width, const unsigned height)
{
     width_ = width;
     height_ = height;
}

void PostEffect::Process(
     ID3D11DeviceContext *deviceContext,
     ID3D11ShaderResourceView *sourceTexture,
     ID3D11RenderTargetView *renderTarget,
     D3D11_VIEWPORT viewport)
{
     ConstBuffer constBuffer_;
     constBuffer_.params = DirectX::XMINT4(useSobel_, useGray_, 0, 0);
     constBuffer_.size = DirectX::XMFLOAT4(1 / static_cast<float>(width_), 1 / static_cast<float>(height_), 0, 0);
     deviceContext->UpdateSubresource(pConstBuffer_, 0, NULL, &constBuffer_, 0, 0);

     deviceContext->OMSetRenderTargets(1, &renderTarget, nullptr);
     deviceContext->RSSetViewports(1, &viewport);

     deviceContext->IASetInputLayout(nullptr);
     deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

     deviceContext->VSSetShader(pVertexShader_, nullptr, 0);
     deviceContext->PSSetShader(pPixelShader_, nullptr, 0);
     deviceContext->PSSetConstantBuffers(0, 1, &pConstBuffer_);
     deviceContext->PSSetShaderResources(0, 1, &sourceTexture);
     deviceContext->PSSetSamplers(0, 1, &pSamplerState_);

     deviceContext->Draw(3, 0);

     ID3D11ShaderResourceView *nullsrv[] = {nullptr};
     deviceContext->PSSetShaderResources(0, 1, nullsrv);
}
