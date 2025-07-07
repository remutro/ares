
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>

struct D3D11Device {

public:

  D3D11Device() {};
  ~D3D11Device() { destroyDevice(); }

  auto createDevice(HWND context, const u32 windowWidth, const u32 windowHeight) -> bool;
  auto destroyDevice(void) -> void;
  auto render(void) -> bool;
  auto generateSwapChain(HWND context, const u32 windowWidth, const u32 windowHeight) -> HRESULT;
  auto compileShaderFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut) -> HRESULT;

  HRESULT hr = S_OK;
  D3D_FEATURE_LEVEL       _featureLevel = D3D_FEATURE_LEVEL_11_1;
  ID3D11Device*           _pd3dDevice = nullptr;
  ID3D11Device1*          _pd3dDevice1 = nullptr;
  ID3D11DeviceContext*    _pImmediateContext = nullptr;
  ID3D11DeviceContext1*   _pImmediateContext1 = nullptr;
  IDXGISwapChain*         _pSwapChain = nullptr;
  IDXGISwapChain1*        _pSwapChain1 = nullptr;
  ID3D11RenderTargetView* _pRenderTargetView = nullptr;

  ID3D11VertexShader*     _pVertexShader = nullptr;
  ID3D11PixelShader*      _pPixelShader = nullptr;
  ID3D11InputLayout*      _pVertexLayout = nullptr;
  ID3D11Buffer*           _pVertexBuffer = nullptr;

};

struct SimpleVertex
{
  DirectX::XMFLOAT3 Pos;
};
