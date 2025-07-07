
auto D3D11Device::createDevice(HWND context, const u32 windowWidth, const u32 windowHeight) -> bool {
    
    D3D_FEATURE_LEVEL d3dFeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE( d3dFeatureLevels );
    UINT layerFlags = 0;

    if(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, layerFlags, d3dFeatureLevels, numFeatureLevels,
                         D3D11_SDK_VERSION, &_pd3dDevice, &_featureLevel, &_pImmediateContext) == E_INVALIDARG) {
        // DirectX 11.0 systems won't recognize feature level 11_1 so retry at level 11_0
        if(FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, layerFlags, &d3dFeatureLevels[1], numFeatureLevels - 1,
                                    D3D11_SDK_VERSION, &_pd3dDevice, &_featureLevel, &_pImmediateContext))) { 
          MessageBox( nullptr, L"Failed to create device.", L"Error", MB_OK );
          return false; 
        }
    }
  
    if(FAILED(generateSwapChain(context, windowWidth, windowHeight))) { 
      MessageBox( nullptr, L"Swap chain creation failure", L"Error", MB_OK );
      return false; }

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &pBackBuffer ) );
    if(FAILED(hr)) { 
      MessageBox( nullptr, L"Swap chain get buffer failure", L"Error", MB_OK );
      return false; }

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();
    if(FAILED(hr)) { 
      MessageBox( nullptr, L"Failed creating render target view", L"Error", MB_OK );
      return false; }

    _pImmediateContext->OMSetRenderTargets( 1, &_pRenderTargetView, nullptr );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)windowWidth;
    vp.Height = (FLOAT)windowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports( 1, &vp );

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = compileShaderFile( L"shader-types.fxh", "VS", "vs_4_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( nullptr,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return false;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
    MessageBox( nullptr, L"Can't create vertex shader.", L"Error", MB_OK );
        return false;
	}

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &_pVertexLayout );
	pVSBlob->Release();
	if( FAILED( hr ) ) {
    MessageBox( nullptr, L"Can't create input layer.", L"Error", MB_OK );
        return false;
  }

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = compileShaderFile( L"shader-types.fxh", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( nullptr,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return false;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader );
	pPSBlob->Release();
    if( FAILED( hr ) ) {
      MessageBox( nullptr, L"Can't create pixel shader.", L"Error", MB_OK );
        return false;
    }

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        DirectX::XMFLOAT3( 0.0f, 0.5f, 0.5f ),
        DirectX::XMFLOAT3( 0.5f, -0.5f, 0.5f ),
        DirectX::XMFLOAT3( -0.5f, -0.5f, 0.5f ),
    };
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 3;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = vertices;
    hr = _pd3dDevice->CreateBuffer( &bd, &InitData, &_pVertexBuffer );
    if( FAILED( hr ) ) {
      MessageBox( nullptr, L"Can't create buffer.", L"Error", MB_OK );
          return false;
    }

    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers( 0, 1, &_pVertexBuffer, &stride, &offset );

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    return true;
}

auto D3D11Device::destroyDevice(void) -> void {

  if( _pImmediateContext ) _pImmediateContext->ClearState();

  if( _pRenderTargetView ) _pRenderTargetView->Release();
  if( _pSwapChain1 ) _pSwapChain1->Release();
  if( _pSwapChain ) _pSwapChain->Release();
  if( _pImmediateContext1 ) _pImmediateContext1->Release();
  if( _pImmediateContext ) _pImmediateContext->Release();
  if( _pd3dDevice1 ) _pd3dDevice1->Release();
  if( _pd3dDevice ) _pd3dDevice->Release();

  if( _pVertexBuffer ) _pVertexBuffer->Release();
  if( _pVertexLayout ) _pVertexLayout->Release();
  if( _pVertexShader ) _pVertexShader->Release();
  if( _pPixelShader ) _pPixelShader->Release();
}

auto D3D11Device::render(void) -> bool {
    FLOAT colorRGBABlack[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    _pImmediateContext->ClearRenderTargetView( _pRenderTargetView, colorRGBABlack );

    // Render a triangle
	_pImmediateContext->VSSetShader( _pVertexShader, nullptr, 0 );
	_pImmediateContext->PSSetShader( _pPixelShader, nullptr, 0 );
    _pImmediateContext->Draw( 3, 0 );

    // Present the information rendered to the back buffer to the front buffer (the screen)
    _pSwapChain->Present( 0, 0 );

    return true; //Fix me
}

auto D3D11Device::compileShaderFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut) -> HRESULT {
    HRESULT hr = S_OK;
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile( szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
    if( FAILED(hr) )
    {
        if( pErrorBlob )
        {
            OutputDebugStringA( reinterpret_cast<const char*>( pErrorBlob->GetBufferPointer() ) );
            pErrorBlob->Release();
        }
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}

auto D3D11Device::generateSwapChain(HWND context, const u32 windowWidth, const u32 windowHeight) -> HRESULT {

    IDXGIFactory1* dxgiFactory = nullptr;
    {
      IDXGIDevice* dxgiDevice = nullptr;
      hr = _pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
      if(SUCCEEDED(hr)) {
          IDXGIAdapter* adapter = nullptr;
          hr = dxgiDevice->GetAdapter(&adapter);
          if(SUCCEEDED(hr)) {
              hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
              adapter->Release();
          }
          dxgiDevice->Release();
      }
    }

    if(FAILED(hr)) { 
      MessageBox( nullptr, L"Swap chain step 1 failed", L"Error", MB_OK );
      return hr; }

    // Swap chain creation
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface( __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2) );
    if(dxgiFactory2) {
        // DirectX 11.1 or later
        hr = _pd3dDevice->QueryInterface( __uuidof(ID3D11Device1), reinterpret_cast<void**>(&_pd3dDevice1) );
        if(SUCCEEDED(hr)) {
            (void) _pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&_pImmediateContext1));
        }

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = windowWidth;
        sd.Height = windowHeight;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd(_pd3dDevice, context, &sd, nullptr, nullptr, &_pSwapChain1);
        if(SUCCEEDED(hr)) {
            hr = _pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&_pSwapChain));
        }

        dxgiFactory2->Release();
    } else {
        // DirectX 11.0 systems
        DXGI_SWAP_CHAIN_DESC scd = {};
        scd.BufferCount = 1;
        scd.BufferDesc.Width = windowWidth;
        scd.BufferDesc.Height = windowHeight;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.RefreshRate.Numerator = 60;
        scd.BufferDesc.RefreshRate.Denominator = 1;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = context;
        scd.SampleDesc.Count = 1;
        scd.SampleDesc.Quality = 0;
        scd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain(_pd3dDevice, &scd, &_pSwapChain);
    }

     // Full-screen swapchains not implemented yet, so no ALT+ENTER for now
    dxgiFactory->MakeWindowAssociation(context, DXGI_MWA_NO_ALT_ENTER);
    dxgiFactory->Release();

    return hr;
  }
