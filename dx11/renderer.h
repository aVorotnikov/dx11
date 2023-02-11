#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>

class Renderer
{
public:
     Renderer();
     ~Renderer();
     void ReleaseAll();

     bool Init(const HWND hWnd);
     bool Render();
     bool Resize(const unsigned width, const unsigned height);

     static constexpr const unsigned defaultWidth = 1280;
     static constexpr const unsigned defaultHeight = 720;
private:
     ID3D11Device *pDevice_;
     ID3D11DeviceContext *pDeviceContext_;
     IDXGISwapChain *pSwapChain_;
     ID3D11RenderTargetView *pBackBufferRTV_;

     unsigned width_;
     unsigned height_;
};
