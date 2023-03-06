#pragma once

#include <d3d11.h>
#include <windows.h>

template <class DirectXClass>
void SafeRelease(DirectXClass *pointer)
{
     if (NULL != pointer)
          pointer->Release();
}

HRESULT CompileShaderFromFile(const WCHAR *szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob **ppBlobOut);
