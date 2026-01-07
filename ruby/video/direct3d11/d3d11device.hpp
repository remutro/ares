
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <vector>

using Microsoft::WRL::ComPtr; 

struct D3D11Device {

public:

  D3D11Device() {};
  ~D3D11Device() {};

  auto createDeviceAndSwapChain(HWND context, const u32 windowWidth, const u32 windowHeight) -> bool;
  auto createRenderTarget(u32 width, u32 height) -> bool;
  auto compileShaders(void) -> bool;
  auto createGeometry(void) -> bool;
  auto createSampler(void) -> bool;
  auto createTextureAndSRV(u32 width, u32 height) -> bool;
  auto updateTexturefromBuffer(u32 width, u32 height, float seconds) -> bool;
  auto render(void) -> void;
  auto releaseTextureBufferResources(void) -> void { _pRenderTargetView.Reset(); }

  HRESULT hr = S_OK;
  
  ComPtr<ID3D11Device>               _pDevice;
  ComPtr<ID3D11DeviceContext>        _pDeviceContext;
  ComPtr<IDXGISwapChain>             _pSwapChain;
  ComPtr<ID3D11RenderTargetView>     _pRenderTargetView;
  ComPtr<ID3D11VertexShader>         _pVertexShader;
  ComPtr<ID3D11PixelShader>          _pPixelShader;
  ComPtr<ID3D11InputLayout>          _pInputLayout;
  ComPtr<ID3D11Buffer>               _pVertexBuffer;
  ComPtr<ID3D11Buffer>               _pIndexBuffer;
  ComPtr<ID3D11ShaderResourceView>   _pTextureSRV;
  ComPtr<ID3D11SamplerState>         _pSamplerState;

  // CPU buffer and texture size
  int g_TexWidth = 800;
  int g_TexHeight = 600;
  std::vector<uint32_t> g_CPUBuffer;

  struct Vertex
  {
    float x, y, z;
    float u, v;
  };
  
};
