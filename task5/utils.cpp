#include "utils.h"
#include <d3dcompiler.h>

HRESULT CompileShaderFromFile(const WCHAR *szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob **ppBlobOut)
{
     DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
     dwShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

     ID3DBlob *pErrorBlob = NULL;
     auto hr = D3DCompileFromFile(
          szFileName,
          NULL,
          NULL,
          szEntryPoint,
          szShaderModel,
          dwShaderFlags,
          0,
          ppBlobOut,
          &pErrorBlob);
     if (FAILED(hr))
     {
          if (pErrorBlob)
          {
               OutputDebugStringA(reinterpret_cast<const char *>(pErrorBlob->GetBufferPointer()));
               pErrorBlob->Release();
          }
          return hr;
     }

     SafeRelease(pErrorBlob);
     return S_OK;
}