#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>
#include <array>

class Renderer
{
public:
     static Renderer &GetInstance();
     Renderer(const Renderer &) = delete;
     Renderer(Renderer &&) = delete;

     ~Renderer();
     void CleanAll();

     bool Init(const HWND hWnd);
     bool Render();
     bool Resize(const unsigned width, const unsigned height);

     static constexpr const unsigned defaultWidth = 1280;
     static constexpr const unsigned defaultHeight = 720;

private:
     Renderer();

     ID3D11Device *pDevice_;
     ID3D11DeviceContext *pDeviceContext_;
     IDXGISwapChain *pSwapChain_;
     ID3D11RenderTargetView *pBackBufferRTV_;

     ID3D11VertexShader *pVertexShader_;
     ID3D11PixelShader *pPixelShader_;
     ID3D11InputLayout *pInputLayout_;
     ID3D11Buffer *pVertexBuffer_;
     ID3D11Buffer *pIndexBuffer_;

     unsigned width_;
     unsigned height_;
};
