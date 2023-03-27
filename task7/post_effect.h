#pragma once

#include <d3d11.h>

class PostEffect
{
public:
     PostEffect(ID3D11Device *device, HWND hwnd, const unsigned width, const unsigned height);
     ~PostEffect();
     void Resize(const unsigned width, const unsigned height);
     void Process(
          ID3D11DeviceContext *deviceContext,
          ID3D11ShaderResourceView *sourceTexture,
          ID3D11RenderTargetView *renderTarget,
          D3D11_VIEWPORT viewport);

private:
     static constexpr const bool defaultUseSobel_ = true;
     static constexpr const bool defaultUseGray_ = false;

     const bool useSobel_;
     const bool useGray_;

     unsigned width_;
     unsigned height_;

     ID3D11VertexShader *pVertexShader_;
     ID3D11PixelShader *pPixelShader_;
     ID3D11SamplerState *pSamplerState_;
     ID3D11Buffer *pConstBuffer_;
};
