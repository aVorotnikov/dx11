#include "renderer.h"
#include "utils.h"

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <chrono>
#include <string>
#include <cmath>

namespace
{

     struct Vertex
     {
          DirectX::XMFLOAT3 pos;
          DirectX::XMFLOAT2 uv;
          DirectX::XMFLOAT3 normal;
          DirectX::XMFLOAT3 tangent;
     };

     struct VertexPos
     {
          float x, y, z;
     };

     struct SceneBuffer
     {
          DirectX::XMMATRIX viewProjMatrix;
          DirectX::XMFLOAT4 cameraPosition;
          int lightCount[4];
          DirectX::XMFLOAT4 lightPositions[maxLightNumber];
          DirectX::XMFLOAT4 lightColors[maxLightNumber];
          DirectX::XMFLOAT4 ambientColor;
     };

     struct WorldBuffer
     {
          DirectX::XMMATRIX worldMatrix;
          DirectX::XMFLOAT4 shine;
     };

     struct TransparentWorldBuffer
     {
          DirectX::XMMATRIX worldMatrix;
          DirectX::XMFLOAT4 color;
     };

     HRESULT SetResourceName(ID3D11DeviceChild *pResource, const std::string &name)
     {
          return pResource->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.length(), name.c_str());
     }

     static const std::array<Vertex, 24> cubeVertices =
     {
          Vertex{{-0.5, -0.5, 0.5}, {0, 1}, {0, -1, 0}, {1, 0, 0}},
          Vertex{{0.5, -0.5, 0.5}, {1, 1}, {0, -1, 0}, {1, 0, 0}},
          Vertex{{0.5, -0.5, -0.5}, {1, 0}, {0, -1, 0}, {1, 0, 0}},
          Vertex{{-0.5, -0.5, -0.5}, {0, 0}, {0, -1, 0}, {1, 0, 0}},

          Vertex{{-0.5, 0.5, -0.5}, {0, 1}, {0, 1, 0}, {1, 0, 0}},
          Vertex{{0.5, 0.5, -0.5}, {1, 1}, {0, 1, 0}, {1, 0, 0}},
          Vertex{{0.5, 0.5, 0.5}, {1, 0}, {0, 1, 0}, {1, 0, 0}},
          Vertex{{-0.5, 0.5, 0.5}, {0, 0}, {0, 1, 0}, {1, 0, 0}},

          Vertex{{0.5, -0.5, -0.5}, {0, 1}, {1, 0, 0}, {0, 0, 1}},
          Vertex{{0.5, -0.5, 0.5}, {1, 1}, {1, 0, 0}, {0, 0, 1}},
          Vertex{{0.5, 0.5, 0.5}, {1, 0}, {1, 0, 0}, {0, 0, 1}},
          Vertex{{0.5, 0.5, -0.5}, {0, 0}, {1, 0, 0}, {0, 0, 1}},

          Vertex{{-0.5, -0.5, 0.5}, {0, 1}, {-1, 0, 0}, {0, 0, -1}},
          Vertex{{-0.5, -0.5, -0.5}, {1, 1}, {-1, 0, 0}, {0, 0, -1}},
          Vertex{{-0.5, 0.5, -0.5}, {1, 0}, {-1, 0, 0}, {0, 0, -1}},
          Vertex{{-0.5, 0.5, 0.5}, {0, 0}, {-1, 0, 0}, {0, 0, -1}},

          Vertex{{0.5, -0.5, 0.5}, {0, 1}, {0, 0, 1}, {-1, 0, 0}},
          Vertex{{-0.5, -0.5, 0.5}, {1, 1}, {0, 0, 1}, {-1, 0, 0}},
          Vertex{{-0.5, 0.5, 0.5}, {1, 0}, {0, 0, 1}, {-1, 0, 0}},
          Vertex{{0.5, 0.5, 0.5}, {0, 0}, {0, 0, 1}, {-1, 0, 0}},

          Vertex{{-0.5, -0.5, -0.5}, {0, 1}, {0, 0, -1}, {1, 0, 0}},
          Vertex{{0.5, -0.5, -0.5}, {1, 1}, {0, 0, -1}, {1, 0, 0}},
          Vertex{{0.5, 0.5, -0.5}, {1, 0}, {0, 0, -1}, {1, 0, 0}},
          Vertex{{-0.5, 0.5, -0.5}, {0, 0}, {0, 0, -1}, {1, 0, 0}}
     };

     static const std::array<USHORT, 36> cubeIndices =
     {
          0, 2, 1, 0, 3, 2,
          4, 6, 5, 4, 7, 6,
          8, 10, 9, 8, 11, 10,
          12, 14, 13, 12, 15, 14,
          16, 18, 17, 16, 19, 18,
          20, 22, 21, 20, 23, 22
     };

     static const std::array<VertexPos, 4> coloredPlaneVertices =
     {
          VertexPos{-1.0, -1.0, 0},
          VertexPos{-1.0, 1.0, 0},
          VertexPos{1.0, 1.0, 0},
          VertexPos{1.0, -1.0, 0},
     };

     static const std::array<USHORT, 6> coloredPlaneIndices =
     {
          0, 2, 1, 0, 3, 2,
     };

}

Renderer &Renderer::GetInstance() {
     static Renderer instance;
     return instance;
}

Renderer::Renderer() :
     pDevice_(NULL),
     pDeviceContext_(NULL),
     pSwapChain_(NULL),
     pBackBufferRTV_(NULL),
     pDepthBuffer_(NULL),
     pDepthBufferDSV_(NULL),
     pVertexShader_(NULL),
     pPixelShader_(NULL),
     pInputLayout_(NULL),
     pVertexBuffer_(NULL),
     pIndexBuffer_(NULL),
     pWorldBuffer_(NULL),
     pWorldBuffer1_(NULL),
     pSceneBuffer_(NULL),
     pRasterizerState_(NULL),
     pDepthState_(NULL),
     pTransparentVertexShader_(NULL),
     pTransparentPixelShader_(NULL),
     pTransparentInputLayout_(NULL),
     pTransparentVertexBuffer_(NULL),
     pTransparentIndexBuffer_(NULL),
     pTransparentWorldBuffer_(NULL),
     pTransparentWorldBuffer1_(NULL),
     pTransparentSceneBuffer_(NULL),
     pTransparentRasterizerState_(NULL),
     pTransparentDepthState_(NULL),
     pTransparentBlendState_(NULL),
     pCubeTexture_(nullptr),
     pCubeNormalMap_(nullptr),
     pCubeMap_(nullptr),
     pCamera_(nullptr),
     pInput_(nullptr),
     width_(defaultWidth),
     height_(defaultHeight),
     start_(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
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

     SafeRelease(pTransparentBlendState_);
     SafeRelease(pTransparentDepthState_);
     SafeRelease(pTransparentRasterizerState_);
     SafeRelease(pTransparentSceneBuffer_);
     SafeRelease(pTransparentWorldBuffer1_);
     SafeRelease(pTransparentWorldBuffer_);
     SafeRelease(pTransparentIndexBuffer_);
     SafeRelease(pTransparentVertexBuffer_);
     SafeRelease(pTransparentInputLayout_);
     SafeRelease(pTransparentPixelShader_);
     SafeRelease(pTransparentVertexShader_);
     SafeRelease(pDepthState_);
     SafeRelease(pRasterizerState_);
     SafeRelease(pSceneBuffer_);
     SafeRelease(pWorldBuffer1_);
     SafeRelease(pWorldBuffer_);
     SafeRelease(pIndexBuffer_);
     SafeRelease(pVertexBuffer_);
     SafeRelease(pInputLayout_);
     SafeRelease(pPixelShader_);
     SafeRelease(pVertexShader_);
     SafeRelease(pDepthBuffer_);
     SafeRelease(pDepthBufferDSV_);
     SafeRelease(pBackBufferRTV_);
     SafeRelease(pDevice_);
     SafeRelease(pDeviceContext_);
     SafeRelease(pSwapChain_);
}

bool Renderer::Init(const HWND hWnd, std::shared_ptr<Camera> pCamera, std::shared_ptr<Input> pInput)
{
     pCamera_ = pCamera;
     pInput_ = pInput;

     // Create a DirectX graphics interface factory.​
     IDXGIFactory *pFactory = nullptr;
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
     result = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackBuffer);
     if (!SUCCEEDED(result))
          return false;
     result = pDevice_->CreateRenderTargetView(pBackBuffer, NULL, &pBackBufferRTV_);
     SafeRelease(pBackBuffer);
     if (!SUCCEEDED(result))
          return false;

     D3D11_TEXTURE2D_DESC textureDesc;
     textureDesc.Format = DXGI_FORMAT_D16_UNORM;
     textureDesc.ArraySize = 1;
     textureDesc.MipLevels = 1;
     textureDesc.Usage = D3D11_USAGE_DEFAULT;
     textureDesc.Height = height_;
     textureDesc.Width = width_;
     textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
     textureDesc.CPUAccessFlags = 0;
     textureDesc.MiscFlags = 0;
     textureDesc.SampleDesc.Count = 1;
     textureDesc.SampleDesc.Quality = 0;

     result = pDevice_->CreateTexture2D(&textureDesc, NULL, &pDepthBuffer_);
     if (FAILED(result))
          return false;
     result = pDevice_->CreateDepthStencilView(pDepthBuffer_, NULL, &pDepthBufferDSV_);
     if (FAILED(result))
     {
          SafeRelease(pDepthBuffer_);
          return false;
     }

     pDeviceContext_->OMSetRenderTargets(1, &pBackBufferRTV_, pDepthBufferDSV_);

     // Compile the vertex shader
     ID3DBlob *pVertexShaderBlob = NULL;
     result = CompileShaderFromFile(L"cube_vertex.hlsl", "main", "vs_5_0", &pVertexShaderBlob);
     if (FAILED(result))
     {
          MessageBox(NULL, L"Failed to compile vertex shader", L"Error", MB_OK);
          return false;
     }

     // Create the vertex shader
     result = pDevice_->CreateVertexShader(
          pVertexShaderBlob->GetBufferPointer(),
          pVertexShaderBlob->GetBufferSize(),
          NULL,
          &pVertexShader_);
     if (FAILED(result))
     {
          pVertexShaderBlob->Release();
          return false;
     }

     // Define the input layout
     D3D11_INPUT_ELEMENT_DESC layout[] =
     {
          {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
          {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
          {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
          {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
     };
     UINT numElements = ARRAYSIZE(layout);

     // Create the input layout
     result = pDevice_->CreateInputLayout(
          layout,
          numElements,
          pVertexShaderBlob->GetBufferPointer(),
          pVertexShaderBlob->GetBufferSize(),
          &pInputLayout_);
     pVertexShaderBlob->Release();
     if (FAILED(result))
          return false;

     // Set the input layout
     pDeviceContext_->IASetInputLayout(pInputLayout_);

     // Compile the pixel shader
     ID3DBlob *pPixelShaderBlob = NULL;
     result = CompileShaderFromFile(L"cube_pixel.hlsl", "main", "ps_5_0", &pPixelShaderBlob);
     if (FAILED(result))
     {
          MessageBox(NULL, L"Failed to compile pixel shader", L"Error", MB_OK);
          return false;
     }

     // Create the pixel shader
     result = pDevice_->CreatePixelShader(
          pPixelShaderBlob->GetBufferPointer(),
          pPixelShaderBlob->GetBufferSize(),
          nullptr,
          &pPixelShader_);
     pPixelShaderBlob->Release();
     if (FAILED(result))
          return false;

     // Create vertex buffer
     D3D11_BUFFER_DESC desc;
     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * cubeVertices.size());
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     D3D11_SUBRESOURCE_DATA data;
     ZeroMemory(&data, sizeof(data));
     data.pSysMem = cubeVertices.data();
     data.SysMemPitch = static_cast<UINT>(sizeof(Vertex) * cubeVertices.size());
     data.SysMemSlicePitch = 0;

     result = pDevice_->CreateBuffer(&desc, &data, &pVertexBuffer_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pVertexBuffer_, "VertexBuffer");
     if (FAILED(result))
          return false;

     // Create index buffer
     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = static_cast<UINT>(sizeof(USHORT) * cubeIndices.size());
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     ZeroMemory(&data, sizeof(data));
     data.pSysMem = cubeIndices.data();
     data.SysMemPitch = static_cast<UINT>(sizeof(USHORT) * cubeIndices.size());;
     data.SysMemSlicePitch = 0;

     result = pDevice_->CreateBuffer(&desc, &data, &pIndexBuffer_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pIndexBuffer_, "IndexBuffer");
     if (FAILED(result))
          return false;

     // Create const buffers
     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = sizeof(WorldBuffer);
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     result = pDevice_->CreateBuffer(&desc, NULL, &pWorldBuffer_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pWorldBuffer_, "WorldBuffer");
     if (FAILED(result))
          return false;

     result = pDevice_->CreateBuffer(&desc, NULL, &pWorldBuffer1_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pWorldBuffer1_, "WorldBuffer1");
     if (FAILED(result))
          return false;

     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = sizeof(SceneBuffer);
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     result = pDevice_->CreateBuffer(&desc, NULL, &pSceneBuffer_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pSceneBuffer_, "SceneBuffer");
     if (FAILED(result))
          return false;

     D3D11_RASTERIZER_DESC rasterizeDesc;
     ZeroMemory(&rasterizeDesc, sizeof(rasterizeDesc));
     rasterizeDesc.AntialiasedLineEnable = false;
     rasterizeDesc.FillMode = D3D11_FILL_SOLID;
     rasterizeDesc.CullMode = D3D11_CULL_BACK;
     rasterizeDesc.DepthBias = 0;
     rasterizeDesc.DepthBiasClamp = 0.0f;
     rasterizeDesc.FrontCounterClockwise = false;
     rasterizeDesc.DepthClipEnable = true;
     rasterizeDesc.ScissorEnable = false;
     rasterizeDesc.MultisampleEnable = false;
     rasterizeDesc.SlopeScaledDepthBias = 0.0f;

     result = pDevice_->CreateRasterizerState(&rasterizeDesc, &pRasterizerState_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pRasterizerState_, "RasterizeBuffer");
     if (FAILED(result))
          return false;

     D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
     ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
     depthStencilDesc.DepthEnable = TRUE;
     depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
     depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
     depthStencilDesc.StencilEnable = FALSE;
     result = pDevice_->CreateDepthStencilState(&depthStencilDesc, &pDepthState_);
     if (FAILED(result))
          return false;

     try
     {
          pCubeTexture_ = std::make_shared<Texture>(pDevice_, cubeTextureFileName_);
          pCubeNormalMap_ = std::make_shared<Texture>(pDevice_, cubeNormalMapFileName_);
          pCubeMap_ = std::make_shared<CubeMap>(pDevice_, pDeviceContext_, width_, height_, fov_, near_);
          pLights_ = std::make_shared<Lights>();

          pLights_->Add(
               {
                    [](std::size_t milliseconds)
                    {
                         return DirectX::XMFLOAT4(0.0f, 1.5f, 2.0f * std::sin(milliseconds / 1000.0f), 0.0f);
                    },
                    [](std::size_t milliseconds)
                    {
                         return DirectX::XMFLOAT4(1.0f, 1.0f, std::sin(milliseconds / 100.0f), 1.0f);
                    }
               });
          pLights_->Add(
               {
                    [](std::size_t milliseconds)
                    {
                         return DirectX::XMFLOAT4(0.0f, -1.5f, -2.0f * std::sin(milliseconds / 300.0f), 0.0f);
                    },
                    [](std::size_t milliseconds)
                    {
                         return DirectX::XMFLOAT4(1.0f, std::sin(milliseconds / 1000.0f), 1.0f, 1.0f);
                    }
               });
          pLights_->Add(
               {
                    [](std::size_t milliseconds)
                    {
                         return DirectX::XMFLOAT4(-1.5f, 0.0f, -2.0f, 0.0f);
                    },
                    [](std::size_t milliseconds)
                    {
                         return DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
                    }
               });
          pLights_->Add(
               {
                    [](std::size_t milliseconds)
                    {
                         return DirectX::XMFLOAT4(1.5f, 0.0f, 2.0f, 0.0f);
                    },
                    [](std::size_t milliseconds)
                    {
                         return DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
                    }
               });
          pLights_->Add(
               {
                    [](std::size_t milliseconds)
                    {
                         return DirectX::XMFLOAT4(std::cos(milliseconds / 2000.0f), std::sin(milliseconds / 500.0f), 0.0f, 0.0f);
                    },
                    [](std::size_t milliseconds)
                    {
                         return DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
                    }
               });
     }
     catch (...)
     {
          return false;
     }

     // Compile the vertex shader
     pVertexShaderBlob = NULL;
     result = CompileShaderFromFile(L"color_vertex.hlsl", "main", "vs_5_0", &pVertexShaderBlob);
     if (FAILED(result))
     {
          MessageBox(NULL, L"Failed to compile vertex shader", L"Error", MB_OK);
          return false;
     }

     // Create the vertex shader
     result = pDevice_->CreateVertexShader(
          pVertexShaderBlob->GetBufferPointer(),
          pVertexShaderBlob->GetBufferSize(),
          NULL,
          &pTransparentVertexShader_);
     if (FAILED(result))
     {
          pVertexShaderBlob->Release();
          return false;
     }

     // Define the input layout
     D3D11_INPUT_ELEMENT_DESC layout1[] =
     {
          {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
     };
     numElements = ARRAYSIZE(layout1);

     // Create the input layout
     result = pDevice_->CreateInputLayout(
          layout1,
          numElements,
          pVertexShaderBlob->GetBufferPointer(),
          pVertexShaderBlob->GetBufferSize(),
          &pTransparentInputLayout_);
     pVertexShaderBlob->Release();
     if (FAILED(result))
          return false;

     // Set the input layout
     pDeviceContext_->IASetInputLayout(pTransparentInputLayout_);

     // Compile the pixel shader
     pPixelShaderBlob = NULL;
     result = CompileShaderFromFile(L"color_pixel.hlsl", "main", "ps_5_0", &pPixelShaderBlob);
     if (FAILED(result))
     {
          MessageBox(NULL, L"Failed to compile pixel shader", L"Error", MB_OK);
          return false;
     }

     // Create the pixel shader
     result = pDevice_->CreatePixelShader(
          pPixelShaderBlob->GetBufferPointer(),
          pPixelShaderBlob->GetBufferSize(),
          nullptr,
          &pTransparentPixelShader_);
     pPixelShaderBlob->Release();
     if (FAILED(result))
          return false;

     // Create vertex buffer
     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = static_cast<UINT>(sizeof(VertexPos) * coloredPlaneVertices.size());
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     ZeroMemory(&data, sizeof(data));
     data.pSysMem = coloredPlaneVertices.data();
     data.SysMemPitch = static_cast<UINT>(sizeof(VertexPos) * coloredPlaneVertices.size());
     data.SysMemSlicePitch = 0;

     result = pDevice_->CreateBuffer(&desc, &data, &pTransparentVertexBuffer_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pTransparentVertexBuffer_, "TransparentVertexBuffer");
     if (FAILED(result))
          return false;

     // Create index buffer
     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = static_cast<UINT>(sizeof(USHORT) * coloredPlaneIndices.size());
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     ZeroMemory(&data, sizeof(data));
     data.pSysMem = coloredPlaneIndices.data();
     data.SysMemPitch = static_cast<UINT>(sizeof(USHORT) * coloredPlaneIndices.size());
     data.SysMemSlicePitch = 0;

     result = pDevice_->CreateBuffer(&desc, &data, &pTransparentIndexBuffer_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pTransparentIndexBuffer_, "TransparentIndexBuffer");
     if (FAILED(result))
          return false;

     // Create const buffers
     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = sizeof(TransparentWorldBuffer);
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     result = pDevice_->CreateBuffer(&desc, NULL, &pTransparentWorldBuffer_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pTransparentWorldBuffer_, "TransparentWorldBuffer");
     if (FAILED(result))
          return false;

     result = pDevice_->CreateBuffer(&desc, NULL, &pTransparentWorldBuffer1_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pTransparentWorldBuffer1_, "TransparentWorldBuffer1");
     if (FAILED(result))
          return false;

     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = sizeof(SceneBuffer);
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     result = pDevice_->CreateBuffer(&desc, NULL, &pTransparentSceneBuffer_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pTransparentSceneBuffer_, "TransparentSceneBuffer");
     if (FAILED(result))
          return false;

     ZeroMemory(&rasterizeDesc, sizeof(rasterizeDesc));
     rasterizeDesc.AntialiasedLineEnable = false;
     rasterizeDesc.FillMode = D3D11_FILL_SOLID;
     rasterizeDesc.CullMode = D3D11_CULL_NONE;
     rasterizeDesc.DepthBias = 0;
     rasterizeDesc.DepthBiasClamp = 0.0f;
     rasterizeDesc.FrontCounterClockwise = false;
     rasterizeDesc.DepthClipEnable = true;
     rasterizeDesc.ScissorEnable = false;
     rasterizeDesc.MultisampleEnable = false;
     rasterizeDesc.SlopeScaledDepthBias = 0.0f;

     result = pDevice_->CreateRasterizerState(&rasterizeDesc, &pTransparentRasterizerState_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pTransparentRasterizerState_, "TransparentRasterizeBuffer");
     if (FAILED(result))
          return false;

     D3D11_BLEND_DESC blendDesc;
     ZeroMemory(&blendDesc, sizeof(blendDesc));
     blendDesc.AlphaToCoverageEnable = false;
     blendDesc.IndependentBlendEnable = false;
     blendDesc.RenderTarget[0].BlendEnable = true;
     blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
     blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
     blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
     blendDesc.RenderTarget[0].RenderTargetWriteMask =
          D3D11_COLOR_WRITE_ENABLE_RED |
          D3D11_COLOR_WRITE_ENABLE_GREEN |
          D3D11_COLOR_WRITE_ENABLE_BLUE;
     blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
     blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
     blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;

     result = pDevice_->CreateBlendState(&blendDesc, &pTransparentBlendState_);
     if (FAILED(result))
          return false;

     ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
     depthStencilDesc.DepthEnable = TRUE;
     depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
     depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
     depthStencilDesc.StencilEnable = FALSE;
     result = pDevice_->CreateDepthStencilState(&depthStencilDesc, &pTransparentDepthState_);
     if (FAILED(result))
          return false;

     return true;
}

bool Renderer::Update()
{
     pInput_->Update();
     pCamera_->Update(pInput_->GetMouseState());

     std::size_t countSec =
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - start_;
     double angle = static_cast<double>(countSec) / 1000;
     WorldBuffer worldBuffer;
     worldBuffer.worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, -1.0f);
     worldBuffer.shine.x = 1000.0f;
     pDeviceContext_->UpdateSubresource(pWorldBuffer_, 0, NULL, &worldBuffer, 0, 0);

     worldBuffer.worldMatrix = DirectX::XMMatrixMultiply(
          DirectX::XMMatrixRotationX(-static_cast<float>(angle)),
          DirectX::XMMatrixTranslation(0.0f, 0.0f, 1.0f));
     pDeviceContext_->UpdateSubresource(pWorldBuffer1_, 0, NULL, &worldBuffer, 0, 0);

     TransparentWorldBuffer transparentWorldBuffer;
     transparentWorldBuffer.worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.1f);
     transparentWorldBuffer.color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 0.5f);
     pDeviceContext_->UpdateSubresource(pTransparentWorldBuffer_, 0, NULL, &transparentWorldBuffer, 0, 0);

     transparentWorldBuffer.worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.1f);
     transparentWorldBuffer.color = DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 0.5f);
     pDeviceContext_->UpdateSubresource(pTransparentWorldBuffer1_, 0, NULL, &transparentWorldBuffer, 0, 0);

     SceneBuffer sceneBuffer;
     const auto view = pCamera_->GetView();
     const auto proj = DirectX::XMMatrixPerspectiveFovLH(fov_, width_ / static_cast<float>(height_), far_, near_);
     sceneBuffer.viewProjMatrix = DirectX::XMMatrixMultiply(view, proj);
     const auto pov = pCamera_->GetPov();
     sceneBuffer.cameraPosition.x = pov.x;
     sceneBuffer.cameraPosition.y = pov.y;
     sceneBuffer.cameraPosition.z = pov.z;
     sceneBuffer.lightCount[0] = static_cast<int>(pLights_->GetNumber());
     sceneBuffer.lightCount[1] = showNormalMap_;
     sceneBuffer.lightCount[2] = showNormals_;
     auto lightPositions = pLights_->GetPositions(countSec);
     std::copy(lightPositions.begin(), lightPositions.end(), sceneBuffer.lightPositions);
     auto lightColors = pLights_->GetColors(countSec);
     std::copy(lightColors.begin(), lightColors.end(), sceneBuffer.lightColors);
     sceneBuffer.ambientColor = ambientColor_;
     pDeviceContext_->UpdateSubresource(pSceneBuffer_, 0, NULL, &sceneBuffer, 0, 0);
     pDeviceContext_->UpdateSubresource(pTransparentSceneBuffer_, 0, NULL, &sceneBuffer, 0, 0);

     pCubeMap_->Update(view, proj, pov);

     return true;
}

bool Renderer::Render()
{
     pDeviceContext_->ClearState();
     ID3D11RenderTargetView *views[] = {pBackBufferRTV_};
     pDeviceContext_->OMSetRenderTargets(1, views, pDepthBufferDSV_);

     static const FLOAT backColor[4] = {0.3f, 0.5f, 0.7f, 1.0f};
     pDeviceContext_->ClearRenderTargetView(pBackBufferRTV_, backColor);
     pDeviceContext_->ClearDepthStencilView(pDepthBufferDSV_, D3D11_CLEAR_DEPTH, 0.0f, 0);

     D3D11_VIEWPORT viewport;
     viewport.TopLeftX = 0;
     viewport.TopLeftY = 0;
     viewport.Width = (FLOAT)width_;
     viewport.Height = (FLOAT)height_;
     viewport.MinDepth = 0.0f;
     viewport.MaxDepth = 1.0f;
     pDeviceContext_->RSSetViewports(1, &viewport);

     D3D11_RECT rect;
     rect.left = 0;
     rect.top = 0;
     rect.right = width_;
     rect.bottom = height_;
     pDeviceContext_->RSSetScissorRects(1, &rect);

     pDeviceContext_->RSSetState(pRasterizerState_);
     pDeviceContext_->OMSetDepthStencilState(pDepthState_, 0);

     ID3D11SamplerState *samplers[] = {pCubeTexture_->GetSampler(), pCubeNormalMap_->GetSampler()};
     pDeviceContext_->PSSetSamplers(0, 2, samplers);

     ID3D11ShaderResourceView *resources[] = {pCubeTexture_->GetTexture(), pCubeNormalMap_->GetTexture()};
     pDeviceContext_->PSSetShaderResources(0, 2, resources);

     pDeviceContext_->IASetIndexBuffer(pIndexBuffer_, DXGI_FORMAT_R16_UINT, 0);
     ID3D11Buffer *vertexBuffers1[] = {pVertexBuffer_};
     UINT strides1[] = {sizeof(Vertex)};
     UINT offsets1[] = {0};
     pDeviceContext_->IASetVertexBuffers(0, 1, vertexBuffers1, strides1, offsets1);
     pDeviceContext_->IASetInputLayout(pInputLayout_);
     pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
     pDeviceContext_->VSSetShader(pVertexShader_, NULL, 0);
     pDeviceContext_->VSSetConstantBuffers(0, 1, &pWorldBuffer_);
     pDeviceContext_->VSSetConstantBuffers(1, 1, &pSceneBuffer_);
     pDeviceContext_->PSSetConstantBuffers(0, 1, &pWorldBuffer_);
     pDeviceContext_->PSSetConstantBuffers(1, 1, &pSceneBuffer_);
     pDeviceContext_->PSSetShader(pPixelShader_, NULL, 0);
     pDeviceContext_->DrawIndexed(static_cast<UINT>(cubeIndices.size()), 0, 0);

     pDeviceContext_->VSSetConstantBuffers(0, 1, &pWorldBuffer1_);
     pDeviceContext_->PSSetConstantBuffers(0, 1, &pWorldBuffer1_);
     pDeviceContext_->DrawIndexed(static_cast<UINT>(cubeIndices.size()), 0, 0);

     pCubeMap_->Render();

     pDeviceContext_->IASetIndexBuffer(pTransparentIndexBuffer_, DXGI_FORMAT_R16_UINT, 0);
     ID3D11Buffer *vertexBuffers[] = {pTransparentVertexBuffer_};
     UINT strides[] = {12};
     UINT offsets[] = {0};
     pDeviceContext_->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
     pDeviceContext_->IASetInputLayout(pTransparentInputLayout_);
     pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
     pDeviceContext_->RSSetState(pTransparentRasterizerState_);
     pDeviceContext_->VSSetShader(pTransparentVertexShader_, NULL, 0);

     pDeviceContext_->OMSetBlendState(pTransparentBlendState_, NULL, 0xFFFFFFFF);
     pDeviceContext_->OMSetDepthStencilState(pTransparentDepthState_, 0);

     pDeviceContext_->VSSetConstantBuffers(1, 1, &pTransparentSceneBuffer_);
     pDeviceContext_->PSSetShader(pTransparentPixelShader_, NULL, 0);

     pDeviceContext_->VSSetConstantBuffers(0, 1, &pTransparentWorldBuffer_);
     pDeviceContext_->PSSetConstantBuffers(0, 1, &pTransparentWorldBuffer_);
     pDeviceContext_->DrawIndexed(static_cast<UINT>(coloredPlaneIndices.size()), 0, 0);

     pDeviceContext_->VSSetConstantBuffers(0, 1, &pTransparentWorldBuffer1_);
     pDeviceContext_->PSSetConstantBuffers(0, 1, &pTransparentWorldBuffer1_);
     pDeviceContext_->DrawIndexed(static_cast<UINT>(coloredPlaneIndices.size()), 0, 0);

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
     SafeRelease(pBackBufferRTV_);

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

     D3D11_TEXTURE2D_DESC textureDesc;
     textureDesc.Format = DXGI_FORMAT_D16_UNORM;
     textureDesc.ArraySize = 1;
     textureDesc.MipLevels = 1;
     textureDesc.Usage = D3D11_USAGE_DEFAULT;
     textureDesc.Height = height;
     textureDesc.Width = width;
     textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
     textureDesc.CPUAccessFlags = 0;
     textureDesc.MiscFlags = 0;
     textureDesc.SampleDesc.Count = 1;
     textureDesc.SampleDesc.Quality = 0;

     SafeRelease(pDepthBuffer_);
     SafeRelease(pDepthBufferDSV_);
     result = pDevice_->CreateTexture2D(&textureDesc, NULL, &pDepthBuffer_);
     if (FAILED(result))
          return false;
     result = pDevice_->CreateDepthStencilView(pDepthBuffer_, NULL, &pDepthBufferDSV_);
     if (FAILED(result))
     {
          SafeRelease(pDepthBuffer_);
          return false;
     }

     pDeviceContext_->OMSetRenderTargets(1, &pBackBufferRTV_, pDepthBufferDSV_);

     width_ = width;
     height_ = height;

     pCubeMap_->Resize(width, height);

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
