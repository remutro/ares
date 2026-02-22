
#include "direct3d11/d3d11device.hpp"

typedef std::unique_ptr<D3D11Device> PD3D11Device;
libra_instance_t _libra;
libra_shader_preset_t _preset = nullptr;
libra_d3d11_filter_chain_t _chain = nullptr;

static LRESULT CALLBACK VideoDirect3D11_WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if(msg == WM_SYSKEYDOWN && wparam == VK_F4) return false;
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

struct VideoDirect3D11 : VideoDriver {
  VideoDirect3D11& self = *this;
  VideoDirect3D11(Video& super) : VideoDriver(super) { construct(); }
  ~VideoDirect3D11() { terminate(); }

  auto create() -> bool override { return initialize(); }
  auto driver() -> string override { return "Direct3D 11.1"; }
 
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
    if(_device) _device->clearRTV();
  }

  auto size(u32& width, u32& height) -> void override {
    if(self.fullScreen) {
      width = _monitorWidth;
      height = _monitorHeight;
    } else {
      RECT rectangle;
      GetClientRect(_context, &rectangle);
      width = rectangle.right - rectangle.left;
      height = rectangle.bottom - rectangle.top;
    }
  }

  auto acquire(u32*& data, u32& pitch, u32 width, u32 height) -> bool override {
    if(!_device || width == 0 || height == 0) return false;

    if(!(_device->createTextureAndSampler(width, height))) return false;

    pitch = _device->getMappedResource().RowPitch;
    return data = (u32*)(_device->getMappedResource().pData);
  }

  auto release() -> void override {
    if(_device) _device->resetRenderTargetView();
  }

  auto output(u32 width, u32 height) -> void override {
    if (!_device || width == 0 || height == 0) return;

    u32 windowWidth = 0, windowHeight = 0;
    size(windowWidth, windowHeight);
    _device->render(width, height, windowWidth, windowHeight);
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

    _device = std::make_unique<D3D11Device>();
  }

  auto updateFilter() -> bool {
    if(!_device) return false;
   
    //acquireContext();
    _device->setShader(self.shader);
    //releaseContext();
    return true;
  }
    
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

    _libra = librashader_load_instance();
    if(!_libra.instance_loaded) {
      print("Direct3D11: Failed to load librashader: shaders will be disabled\n");
    }

    if(!(_device->createDeviceAndSwapChain(_context))) { return false; }
    if(!(_device->createRenderTarget())) { return false; }
    if(!(_device->compileShaders())) { return false; }
    if(!(_device->createGeometry())) { return false; }
    if(!(_device->createSamplerState())) { return false; }
  
    return true;
  }
  
  auto terminate() -> void {
    if(_device) _device->resetRenderTargetView();
    if(_window) { DestroyWindow(_window); _window = nullptr; }
    _context = nullptr;
  }

  PD3D11Device _device = nullptr;
  HWND _window = nullptr;
  HWND _context = nullptr;
  bool _exclusive = false;

  s32 _monitorX = 0;
  s32 _monitorY = 0;
  s32 _monitorWidth = 0;
  s32 _monitorHeight = 0;
};
