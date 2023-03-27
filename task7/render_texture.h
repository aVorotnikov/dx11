#pragma once

#include <d3d11.h>
#include <directxmath.h>

class RenderTexture
{
public:
     RenderTexture(ID3D11Device *device, const unsigned width, const unsigned height);
     ~RenderTexture();
     bool Resize(const unsigned width, const unsigned height);
     void SetRenderTarget(ID3D11DeviceContext *deviceContext, ID3D11DepthStencilView *depthStencilView);
     void ClearRenderTarget(
          ID3D11DeviceContext *deviceContext,
          ID3D11DepthStencilView *depthStencilView,
          const DirectX::XMFLOAT4& color);
     ID3D11Texture2D *GetRT();
     ID3D11RenderTargetView *GetRTV();
     ID3D11ShaderResourceView *GetSRV();
     D3D11_VIEWPORT GetViewPort();

private:
     ID3D11Device *device_;
     ID3D11Texture2D *pRTTexture_;
     ID3D11RenderTargetView *pRTV_;
     ID3D11ShaderResourceView *pSRV_;
     D3D11_VIEWPORT viewport_;
};
