#include "render_texture.h"
#include "utils.h"
#include <exception>
#include <array>

RenderTexture::RenderTexture(ID3D11Device *device, const unsigned width, const unsigned height) :
     device_(device), pRTTexture_(nullptr), pRTV_(nullptr), pSRV_(nullptr)
{
     if (!Resize(width, height))
          throw std::exception("Failed to resize render target");
}

RenderTexture::~RenderTexture()
{
     SafeRelease(pSRV_);
     SafeRelease(pRTV_);
     SafeRelease(pRTTexture_);
}

bool RenderTexture::Resize(const unsigned width, const unsigned height)
{
     D3D11_TEXTURE2D_DESC textureDesc;
     ZeroMemory(&textureDesc, sizeof(textureDesc));

     textureDesc.Width = width;
     textureDesc.Height = height;
     textureDesc.MipLevels = 1;
     textureDesc.ArraySize = 1;
     textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
     textureDesc.SampleDesc.Count = 1;
     textureDesc.Usage = D3D11_USAGE_DEFAULT;
     textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
     textureDesc.CPUAccessFlags = 0;
     textureDesc.MiscFlags = 0;

     HRESULT result = device_->CreateTexture2D(&textureDesc, NULL, &pRTTexture_);
     if (FAILED(result))
          return false;

     D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
     renderTargetViewDesc.Format = textureDesc.Format;
     renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
     renderTargetViewDesc.Texture2D.MipSlice = 0;

     result = device_->CreateRenderTargetView(pRTTexture_, &renderTargetViewDesc, &pRTV_);
     if (FAILED(result))
          return false;

     D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
     shaderResourceViewDesc.Format = textureDesc.Format;
     shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
     shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
     shaderResourceViewDesc.Texture2D.MipLevels = 1;

     result = device_->CreateShaderResourceView(pRTTexture_, &shaderResourceViewDesc, &pSRV_);
     if (FAILED(result))
          return false;

     viewport_.Width = (FLOAT)width;
     viewport_.Height = (FLOAT)height;
     viewport_.MinDepth = 0.0f;
     viewport_.MaxDepth = 1.0f;
     viewport_.TopLeftX = 0;
     viewport_.TopLeftY = 0;

     return true;
}

void RenderTexture::SetRenderTarget(ID3D11DeviceContext *deviceContext, ID3D11DepthStencilView *depthStencilView)
{
     deviceContext->OMSetRenderTargets(1, &pRTV_, depthStencilView);
}

void RenderTexture::ClearRenderTarget(
     ID3D11DeviceContext *deviceContext,
     ID3D11DepthStencilView *depthStencilView,
     const DirectX::XMFLOAT4 &color)
{
     std::array<float, 4> colorRaw = {color.x, color.y, color.z, color.w};
     deviceContext->ClearRenderTargetView(pRTV_, colorRaw.data());
     deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 0.0f, 0);
}

ID3D11Texture2D *RenderTexture::GetRT()
{
     return pRTTexture_;
}

ID3D11RenderTargetView *RenderTexture::GetRTV()
{
     return pRTV_;
}

ID3D11ShaderResourceView *RenderTexture::GetSRV()
{
     return pSRV_;
}

D3D11_VIEWPORT RenderTexture::GetViewPort()
{
     return viewport_;
}
