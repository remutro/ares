
#include "direct3d11/d3d11device.hpp"

D3D11Device _device;
// Timing
std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();

static LRESULT CALLBACK VideoDirect3D11_WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if(msg == WM_SYSKEYDOWN && wparam == VK_F4) return false;
/*
  if(msg == WM_SIZE) {
    if (_device._pDevice) {
      UINT width = LOWORD(lparam);
      UINT height = HIWORD(lparam);

      // Avoid handling when minimized
      if (width != 0 || height != 0) {
        // Release RTV, resize buffers, recreate RTV and viewport, recreate texture to match new size
        _device.releaseTextureBufferResources();
        HRESULT hr = _device._pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        _device.createRenderTarget(width, height);

        // Recreate texture / SRV to match new size
        _device._pTextureSRV.Reset();
        _device.createTextureAndSRV(width, height);

        // Update viewport to new size
        D3D11_VIEWPORT vp = {};
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width = static_cast<FLOAT>(width);
        vp.Height = static_cast<FLOAT>(height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        _device._pDeviceContext->RSSetViewports(1, &vp);
      }
    }
  }
  */
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

struct VideoDirect3D11 : VideoDriver {
  VideoDirect3D11& self = *this;
  VideoDirect3D11(Video& super) : VideoDriver(super) { construct(); }
  ~VideoDirect3D11() { destruct(); }

  auto create() -> bool override {
    return initialize();
  }

  auto driver() -> string override { return "Direct3D 11.1"; }
  auto ready() -> bool override { return _ready; }

  auto hasFullScreen() -> bool override { return true; }
  auto hasMonitor() -> bool override { return true; }
  auto hasExclusive() -> bool override { return true; }
  auto hasContext() -> bool override { return true; }
  auto hasBlocking() -> bool override { return true; }
  auto hasShader() -> bool override { return false; }

  auto setFullScreen(bool fullScreen) -> bool override { return initialize(); }
  auto setMonitor(string monitor) -> bool override { return initialize(); }
  auto setExclusive(bool exclusive) -> bool override { return initialize(); }
  auto setContext(uintptr context) -> bool override { return initialize(); }
  auto setBlocking(bool blocking) -> bool override { return initialize(); }
  auto setShader(string shader) -> bool override { return updateFilter(); }

  auto focused() -> bool override {
    if(self.fullScreen && self.exclusive) return true;
    auto focused = GetFocus();
    return _context == focused || IsChild(_context, focused);
  }

  auto clear() -> void override {
    if(_lost && !recover()) return;
/*
    D3DSURFACE_DESC surfaceDescription;
    _texture->GetLevelDesc(0, &surfaceDescription);
    _texture->GetSurfaceLevel(0, &_surface);

    if(_surface) {
      D3DLOCKED_RECT lockedRectangle;
      _surface->LockRect(&lockedRectangle, 0, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD);
      memory::fill(lockedRectangle.pBits, lockedRectangle.Pitch * surfaceDescription.Height);
      _surface->UnlockRect();
      _surface->Release();
      _surface = nullptr;
    }

    //clear primary display and all backbuffers
    for(u32 n : range(3)) {
      _device->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0x00, 0x00, 0x00), 1.0f, 0);
      _device->Present(0, 0, 0, 0);
    }
    */
  }

  auto size(u32& width, u32& height) -> void override {
    /*
    if(_lost && !recover()) return;

    RECT rectangle;
    GetClientRect(_context, &rectangle);

    width = rectangle.right - rectangle.left;
    height = rectangle.bottom - rectangle.top;

    //if output size changed, driver must be re-initialized.
    //failure to do so causes scaling issues on some video drivers.
    if(width != _windowWidth || height != _windowHeight) initialize();
    */
  }

  auto acquire(u32*& data, u32& pitch, u32 width, u32 height) -> bool override {
    /*
    if(_lost && !recover()) return false;

    u32 windowWidth, windowHeight;
    size(windowWidth, windowHeight);

    if(width != _inputWidth || height != _inputHeight) {
      resize(_inputWidth = width, _inputHeight = height);
    }

    D3DSURFACE_DESC surfaceDescription;
    _texture->GetLevelDesc(0, &surfaceDescription);
    _texture->GetSurfaceLevel(0, &_surface);

    D3DLOCKED_RECT lockedRectangle;
    _surface->LockRect(&lockedRectangle, 0, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD);
    pitch = lockedRectangle.Pitch;
    */
    return data;// = (u32*)lockedRectangle.pBits;
  }

  auto release() -> void override {
  /*
    _surface->UnlockRect();
    _surface->Release();
    _surface = nullptr;
  */
  }

  auto output(u32 width, u32 height) -> void override {
    if(_lost && !recover()) return;

    if(!width) width = _windowWidth;
    if(!height) height = _windowHeight;

    auto now = std::chrono::high_resolution_clock::now();
    float seconds = std::chrono::duration<float>(now - start).count();

    _device.updateTexturefromBuffer(width, height, seconds);
    _device.render();

    /*
    _device->BeginScene();
    //center output within window
    u32 x = (_windowWidth - width) / 2;
    u32 y = (_windowHeight - height) / 2;
    setVertex(0, 0, _inputWidth, _inputHeight, _textureWidth, _textureHeight, x, y, width, height);
    _device->SetTexture(0, _texture);
    _device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    _device->EndScene();

    if(_device->Present(0, 0, 0, 0) == D3DERR_DEVICELOST) _lost = true;
    */
  }

private:
  auto construct() -> void {
    WNDCLASS windowClass{};
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    windowClass.hInstance = GetModuleHandle(0);
    windowClass.lpfnWndProc = VideoDirect3D11_WindowProcedure;
    windowClass.lpszClassName = L"VideoDirect3D11_Window";
    windowClass.lpszMenuName = 0;
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClass(&windowClass);
  }

  auto destruct() -> void {
    terminate();
  }

  auto recover() -> bool {
/*
    if(!_device) return false;

    if(_lost) {
      if(_vertexBuffer) { _vertexBuffer->Release(); _vertexBuffer = nullptr; }
      if(_surface) { _surface->Release(); _surface = nullptr; }
      if(_texture) { _texture->Release(); _texture = nullptr; }
      if(_device->Reset(&_presentation) != D3D_OK) return false;
    }
    _lost = false;

    _device->SetDialogBoxMode(false);

    _device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    _device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    _device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    _device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    _device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    _device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    _device->SetRenderState(D3DRS_LIGHTING, false);
    _device->SetRenderState(D3DRS_ZENABLE, false);
    _device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    _device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    _device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    _device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

    _device->SetVertexShader(nullptr);
    _device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

    _device->CreateVertexBuffer(sizeof(Vertex) * 4, _vertexUsage, D3DFVF_XYZRHW | D3DFVF_TEX1,
      (D3DPOOL)_vertexPool, &_vertexBuffer, nullptr);
    _textureWidth = 0;
    _textureHeight = 0;
    resize(_inputWidth = 256, _inputHeight = 256);
    updateFilter();
    clear();
*/
    return true;
  }

  auto resize(u32 width, u32 height) -> void {
    if(_textureWidth >= width && _textureHeight >= height) return;

    _textureWidth = bit::round(max(width, _textureWidth));
    _textureHeight = bit::round(max(height, _textureHeight));

/*
    if(_capabilities.MaxTextureWidth < _textureWidth || _capabilities.MaxTextureWidth < _textureHeight) return;

    if(_texture) _texture->Release();
    _device->CreateTexture(_textureWidth, _textureHeight, 1, _textureUsage, D3DFMT_X8R8G8B8,
      (D3DPOOL)_texturePool, &_texture, nullptr);
*/
  }

  auto updateFilter() -> bool {
    //if(!_device) return false;
    //if(_lost && !recover()) return false;

    //_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    //_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    return true;
  }
/*
  //(x,y) screen coordinates, in pixels
  //(u,v) texture coordinates, betweeen 0.0 (top, left) to 1.0 (bottom, right)
  auto setVertex(u32 px, u32 py, u32 pw, u32 ph, u32 tw, u32 th, u32 x, u32 y, u32 w, u32 h) -> void {
    Vertex vertex[4];
    vertex[0].x = vertex[2].x = (f64)(x     - 0.5);
    vertex[1].x = vertex[3].x = (f64)(x + w - 0.5);
    vertex[0].y = vertex[1].y = (f64)(y     - 0.5);
    vertex[2].y = vertex[3].y = (f64)(y + h - 0.5);

    //Z-buffer and RHW are unused for 2D blit, set to normal values
    vertex[0].z = vertex[1].z = vertex[2].z = vertex[3].z = 0.0;
    vertex[0].rhw = vertex[1].rhw = vertex[2].rhw = vertex[3].rhw = 1.0;

    f64 rw = (f64)w / (f64)pw * (f64)tw;
    f64 rh = (f64)h / (f64)ph * (f64)th;
    vertex[0].u = vertex[2].u = (f64)(px    ) / rw;
    vertex[1].u = vertex[3].u = (f64)(px + w) / rw;
    vertex[0].v = vertex[1].v = (f64)(py    ) / rh;
    vertex[2].v = vertex[3].v = (f64)(py + h) / rh;

    LPDIRECT3DVERTEXBUFFER9* vertexPointer = nullptr;
    _vertexBuffer->Lock(0, sizeof(Vertex) * 4, (void**)&vertexPointer, 0);
    memory::copy<Vertex>(vertexPointer, vertex, 4);
    _vertexBuffer->Unlock();

    _device->SetStreamSource(0, _vertexBuffer, 0, sizeof(Vertex));

  }
*/
  auto initialize() -> bool {
    terminate();
    if(!self.fullScreen && !self.context) return false;

    auto monitor = Video::monitor(self.monitor);
    _monitorX = monitor.x;
    _monitorY = monitor.y;
    _monitorWidth = monitor.width;
    _monitorHeight = monitor.height;

    _exclusive = self.exclusive && self.fullScreen;

    //Direct3D exclusive mode targets the primary monitor only
    if(_exclusive) {
      POINT point{0, 0};  //the primary monitor always starts at (0,0)
      HMONITOR monitor = MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY);
      MONITORINFOEX info{};
      info.cbSize = sizeof(MONITORINFOEX);
      GetMonitorInfo(monitor, &info);
      _monitorX = info.rcMonitor.left;
      _monitorY = info.rcMonitor.top;
      _monitorWidth = info.rcMonitor.right - info.rcMonitor.left;
      _monitorHeight = info.rcMonitor.bottom - info.rcMonitor.top;
    }

    if(self.fullScreen) {
      _context = _window = CreateWindowEx(WS_EX_TOPMOST, L"VideoDirect3D11_Window", L"", WS_VISIBLE | WS_POPUP,
        _monitorX, _monitorY, _monitorWidth, _monitorHeight,
        nullptr, nullptr, GetModuleHandle(0), nullptr);
    } else {
      _context = (HWND)self.context;
    }

    RECT rectangle;
    GetClientRect(_context, &rectangle);
    _windowWidth = rectangle.right - rectangle.left;
    _windowHeight = rectangle.bottom - rectangle.top;

    if(!(_device.createDeviceAndSwapChain(_context, _windowWidth, _windowHeight))) { return false; }
    if(!(_device.createRenderTarget(_windowWidth, _windowHeight))) { return false; }
    if(!(_device.compileShaders())) { return false; }
    if(!(_device.createGeometry())) { return false; }
    if(!(_device.createSampler())) { return false; }
    if(!(_device.createTextureAndSRV(_windowWidth, _windowHeight))) { return false; }

    //output(_windowWidth, _windowHeight);

    _lost = false;
    //return _ready = recover();
    return (_ready = true);
  }
  
  auto terminate() -> void {
    _ready = false;
    
    _device.releaseTextureBufferResources();
  
    if(_window) { DestroyWindow(_window); _window = nullptr; }
    _context = nullptr;
  }

  struct VertexCoords {
    float x, y, z, rhw;  //screen coordinates
    float u, v;          //texture coordinates
  };

  bool _ready = false;
  bool _exclusive = false;
  bool _lost = true;

  HWND _window = nullptr;
  HWND _context = nullptr;

  /*
  LPDIRECT3D9 _instance = nullptr;
  LPDIRECT3DDEVICE9 _device = nullptr;
  LPDIRECT3DVERTEXBUFFER9 _vertexBuffer = nullptr;
  D3DPRESENT_PARAMETERS _presentation = {};
  D3DCAPS9 _capabilities = {};
  LPDIRECT3DTEXTURE9 _texture = nullptr;
  LPDIRECT3DSURFACE9 _surface = nullptr;
*/

  u32 _windowWidth;
  u32 _windowHeight;
  u32 _textureWidth;
  u32 _textureHeight;
  s32 _monitorX;
  s32 _monitorY;
  s32 _monitorWidth;
  s32 _monitorHeight;
  u32 _inputWidth;
  u32 _inputHeight;

  u32 _textureUsage;
  u32 _texturePool;
  u32 _vertexUsage;
  u32 _vertexPool;
};
