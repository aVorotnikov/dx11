#pragma once

#include "camera.h"
#include "input.h"
#include "texture.h"
#include "cube_map.h"
#include "lights.h"

#include <d3d11.h>
#include <dxgi.h>
#include <directxmath.h>
#include <windows.h>
#include <array>
#include <memory>

class Renderer
{
public:
     static Renderer &GetInstance();
     Renderer(const Renderer &) = delete;
     Renderer(Renderer &&) = delete;

     ~Renderer();
     void CleanAll();

     bool Init(const HWND hWnd, std::shared_ptr<Camera> pCamera, std::shared_ptr<Input> pInput);
     bool Update();
     bool Render();
     bool Resize(const unsigned width, const unsigned height);

     static constexpr const unsigned defaultWidth = 1280;
     static constexpr const unsigned defaultHeight = 720;

private:
     static constexpr const WCHAR cubeTextureFileName_[] = L"images/brick_diffuse.dds";
     static constexpr const WCHAR cubeNormalMapFileName_[] = L"images/brick_normal.dds";

     static constexpr const float near_ = 0.1f;
     static constexpr const float far_ = 100.0f;
     static constexpr const float fov_ = DirectX::XM_PI / 3;
     static constexpr const DirectX::XMFLOAT4 ambientColor_{0.5f, 0.5f, 0.5f, 1.0f};

     Renderer();

     ID3D11Device *pDevice_;
     ID3D11DeviceContext *pDeviceContext_;
     IDXGISwapChain *pSwapChain_;
     ID3D11RenderTargetView *pBackBufferRTV_;
     ID3D11Texture2D *pDepthBuffer_;
     ID3D11DepthStencilView *pDepthBufferDSV_;

     ID3D11VertexShader *pVertexShader_;
     ID3D11PixelShader *pPixelShader_;
     ID3D11InputLayout *pInputLayout_;
     ID3D11Buffer *pVertexBuffer_;
     ID3D11Buffer *pIndexBuffer_;
     ID3D11Buffer *pWorldBuffer_;
     ID3D11Buffer *pWorldBuffer1_;
     ID3D11Buffer *pSceneBuffer_;
     ID3D11RasterizerState *pRasterizerState_;
     ID3D11DepthStencilState *pDepthState_;

     ID3D11VertexShader *pTransparentVertexShader_;
     ID3D11PixelShader *pTransparentPixelShader_;
     ID3D11InputLayout *pTransparentInputLayout_;
     ID3D11Buffer *pTransparentVertexBuffer_;
     ID3D11Buffer *pTransparentIndexBuffer_;
     ID3D11Buffer *pTransparentWorldBuffer_;
     ID3D11Buffer *pTransparentWorldBuffer1_;
     ID3D11Buffer *pTransparentSceneBuffer_;
     ID3D11RasterizerState *pTransparentRasterizerState_;
     ID3D11DepthStencilState *pTransparentDepthState_;
     ID3D11BlendState *pTransparentBlendState_;

     std::shared_ptr<Texture> pCubeTexture_;
     std::shared_ptr<Texture> pCubeNormalMap_;
     std::shared_ptr<CubeMap> pCubeMap_;
     std::shared_ptr<Lights> pLights_;

     std::shared_ptr<Camera> pCamera_;
     std::shared_ptr<Input> pInput_;

     unsigned width_;
     unsigned height_;

     std::size_t start_;
};
