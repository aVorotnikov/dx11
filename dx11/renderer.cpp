#include "renderer.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib, "dxgi")

namespace
{

template <class DirectXClass>
void SafeRelease(DirectXClass *pointer)
{
     if (NULL != pointer)
          pointer->Release();
}

}

Renderer& Renderer::GetInstance() {
     static Renderer instance;
     return instance;
}

Renderer::Renderer() :
     pDevice_(NULL),
     pDeviceContext_(NULL),
     pSwapChain_(NULL),
     pBackBufferRTV_(NULL),
     width_(defaultWidth),
     height_(defaultHeight)
{
}

Renderer::~Renderer()
{
     CleanAll();
}

void Renderer::CleanAll()
{
     if (NULL != pDeviceContext_)
          pDeviceContext_->ClearState();

     SafeRelease(pBackBufferRTV_);
     SafeRelease(pDevice_);
     SafeRelease(pDeviceContext_);
     SafeRelease(pSwapChain_);
}

bool Renderer::Init(const HWND hWnd)
{
     // Create a DirectX graphics interface factory.​
     IDXGIFactory* pFactory = nullptr;
     HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory);
     // Select hardware adapter
     IDXGIAdapter *pSelectedAdapter = NULL;
     if (SUCCEEDED(result))
     {
          IDXGIAdapter *pAdapter = NULL;
          UINT adapterIdx = 0;
          while (SUCCEEDED(pFactory->EnumAdapters(adapterIdx, &pAdapter)))
          {
               DXGI_ADAPTER_DESC desc;
               pAdapter->GetDesc(&desc);
               if (wcscmp(desc.Description, L"Microsoft Basic Render Driver"))
               {
                    pSelectedAdapter = pAdapter;
                    break;
               }
               pAdapter->Release();
               adapterIdx++;
          }
     }
     if (NULL == pSelectedAdapter)
          return false;

     // Create DirectX11 device
     D3D_FEATURE_LEVEL level;
     D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0};
     UINT flags = 0;
#ifdef _DEBUG
     flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG
     result = D3D11CreateDevice(
          pSelectedAdapter,
          D3D_DRIVER_TYPE_UNKNOWN,
          NULL,
          flags,
          levels,
          1,
          D3D11_SDK_VERSION,
          &pDevice_,
          &level,
          &pDeviceContext_
     );
     if (D3D_FEATURE_LEVEL_11_0 != level || !SUCCEEDED(result))
          return false;

     // Create swap chain
     DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
     swapChainDesc.BufferCount = 2;
     swapChainDesc.BufferDesc.Width = width_;
     swapChainDesc.BufferDesc.Height = height_;
     swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
     swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
     swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
     swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
     swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
     swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
     swapChainDesc.OutputWindow = hWnd;
     swapChainDesc.SampleDesc.Count = 1;
     swapChainDesc.SampleDesc.Quality = 0;
     swapChainDesc.Windowed = true;
     swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
     swapChainDesc.Flags = 0;
     result = pFactory->CreateSwapChain(pDevice_, &swapChainDesc, &pSwapChain_);
     if (!SUCCEEDED(result))
          return false;

     ID3D11Texture2D *pBackBuffer = NULL;
     result = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
     if (!SUCCEEDED(result))
          return false;
     result = pDevice_->CreateRenderTargetView(pBackBuffer, NULL, &pBackBufferRTV_);
     if (!SUCCEEDED(result))
          return false;
     SafeRelease(pBackBuffer);

     return true;
}

bool Renderer::Render()
{
     pDeviceContext_->ClearState();
     ID3D11RenderTargetView *views[] = {pBackBufferRTV_};
     pDeviceContext_->OMSetRenderTargets(1, views, NULL);

     static const FLOAT backColor[4] = {0.3f, 0.5f, 0.7f, 1.0f};
     pDeviceContext_->ClearRenderTargetView(pBackBufferRTV_, backColor);

     HRESULT result = pSwapChain_->Present(0, 0);
     if (!SUCCEEDED(result))
          return false;

     return true;
}

bool Renderer::Resize(const unsigned width, const unsigned height)
{
     if (NULL == pSwapChain_)
          return false;

     pDeviceContext_->OMSetRenderTargets(0, 0, 0);
     pBackBufferRTV_->Release();

     auto result = pSwapChain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
     if (!SUCCEEDED(result))
          return false;

     ID3D11Texture2D *pBuffer;
     result = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&pBuffer));
     if (!SUCCEEDED(result))
          return false;

     result = pDevice_->CreateRenderTargetView(pBuffer, NULL, &pBackBufferRTV_);
     pBuffer->Release();
     if (!SUCCEEDED(result))
          return false;

     pDeviceContext_->OMSetRenderTargets(1, &pBackBufferRTV_, NULL);

     D3D11_VIEWPORT vp;
     vp.Width = (FLOAT)width;
     vp.Height = (FLOAT)height;
     vp.MinDepth = 0.0f;
     vp.MaxDepth = 1.0f;
     vp.TopLeftX = 0;
     vp.TopLeftY = 0;
     pDeviceContext_->RSSetViewports(1, &vp);

     return true;
}
