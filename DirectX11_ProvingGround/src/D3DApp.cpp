#include "D3DApp.h"
#include "D3DUtil.h"
#include "DXTrace.h"
#include <sstream>


extern "C"
{
	// 在具有多显卡的硬件设备中，优先使用 NVIDIA 或 AMD 的显卡运行
	// 需要在 .exe 中使用
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

namespace
{
	// 这只用于从全局窗口转发 Windows 消息
	// 过程转换为成员函数窗口过程，因为不能指定一个成员函数给 WNDCLASS::lpfnWndProc
	D3DApp* g_pd3dApp = nullptr;
}

LRESULT WINAPI MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before m_hMainWnd is valid.
	return g_pd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
	:m_hAppInst(hInstance),
	m_MainWndCaption(windowName),
	m_ClientWidth(initWidth),
	m_ClientHeight(initHeight),
	m_hMainWnd(nullptr),
	m_bAppPaused(false),
	m_bMinimized(false),
	m_bMaximized(false),
	m_bResizing(false),
	m_bEnable4xMsaa(true),
	m_4xMsaaQuality(0),
	m_pd3dDevice(nullptr),
	m_pd3dImmediateContext(nullptr),
	m_pSwapChain(nullptr),
	m_pd3dDevice1(nullptr),
	m_pd3dImmediateContext1(nullptr),
	m_pSwapChain1(nullptr),
	m_pDepthStencilBuffer(nullptr),
	m_pRenderTargetView(nullptr),
	m_pDepthStencilView(nullptr)
{
	ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));

	// 让一个全局指针获取这个类，这样我们就可以在 Windows 消息处理的回调函数
	// 让这个类调用内部的回调函数了
	g_pd3dApp = this;
}

D3DApp::~D3DApp()
{
	if (m_pd3dImmediateContext)
	{
		// 恢复所有默认设定
		m_pd3dImmediateContext->ClearState();
	}
}

int D3DApp::Run()
{
	MSG msg = { 0 };

	m_Timer.Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else
		{
			m_Timer.Tick();

			if (!m_bAppPaused)
			{
				CalculateFrameStats();
				UpdateScene(m_Timer.DeltaTime());
				DrawScene();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return static_cast<int>(msg.wParam);
}

bool D3DApp::Init()
{
	if (!InitMainWindow())
		return false;

	if (!InitDirect3D())
		return false;

	return true;
}

void D3DApp::OnResize()
{
	assert(m_pd3dDevice);
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	if (m_pd3dDevice1 != nullptr)
	{
		assert(m_pd3dDevice1);
		assert(m_pd3dImmediateContext1);
		assert(m_pSwapChain1);
	}

	// 释放渲染管线输出用到的资源
	m_pRenderTargetView.Reset();
	m_pDepthStencilView.Reset();
	m_pDepthStencilBuffer.Reset();

	// 重设交换链并重新创建渲染目标视图
	ComPtr<ID3D11Texture2D> backBuffer;
	HR(m_pSwapChain->ResizeBuffers(1, m_ClientWidth, m_ClientHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0));
	HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf())));
	HR(m_pd3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_pRenderTargetView.GetAddressOf()));

	// 设置对象调试名
	D3D11SetDebugObjectName(backBuffer.Get(), "BackBuffer[0]");

	backBuffer.Reset();


	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.ArraySize = 1;
	// 要使用 4X MSAA?
	if (m_bEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.Width = m_ClientWidth;
	depthStencilDesc.Height = m_ClientHeight;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.MipLevels = 1;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	// 要使用 4X MSAA?
	if (m_bEnable4xMsaa)
	{
		// 这里如果启用多重采样设置了 depthStencilDesc.SampleDesc 需要用 D3D11_DSV_DIMENSION_TEXTURE2DMS 格式
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	}
	else
	{
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	}

	// 创建深度缓冲区以及深度模板视图
	HR(m_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, m_pDepthStencilBuffer.GetAddressOf()));
	HR(m_pd3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), &depthStencilViewDesc, m_pDepthStencilView.GetAddressOf()));


	// 将渲染目标视图和 深度/模板缓冲结合到管线
	m_pd3dImmediateContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

	// 设置视口变换
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
	m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
		// https://learn.microsoft.com/zh-cn/windows/win32/inputdev/wm-activate
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_bAppPaused = true;
			m_Timer.Stop();
		}
		else
		{
			m_bAppPaused = false;
			m_Timer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
		// https://learn.microsoft.com/zh-cn/windows/win32/winmsg/wm-size
	case WM_SIZE:
		// Save the new client area dimensions.
		m_ClientWidth = LOWORD(lParam);
		m_ClientHeight = HIWORD(lParam);
		if (m_pd3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_bAppPaused = true;
				m_bMinimized = true;
				m_bMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_bAppPaused = false;
				m_bMinimized = false;
				m_bMaximized = true;
				OnResize();
			}
			// 窗口已调整大小，但 SIZE_MINIMIZED 和 SIZE_MAXIMIZED 值都不适用。
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (m_bMinimized)
				{
					m_bAppPaused = false;
					m_bMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (m_bMaximized)
				{
					m_bAppPaused = false;
					m_bMaximized = false;
					OnResize();
				}
				else if (m_bResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or m_pSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

		// WM_ENTERSIZEMOVE is sent when the user grabs the resize bars.
		// https://learn.microsoft.com/zh-cn/windows/win32/winmsg/wm-entersizemove
	case WM_ENTERSIZEMOVE:
		m_bAppPaused = true;
		m_bResizing = true;
		m_Timer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
		// https://learn.microsoft.com/zh-cn/windows/win32/winmsg/wm-exitsizemove
	case WM_EXITSIZEMOVE:
		m_bAppPaused = false;
		m_bResizing = false;
		m_Timer.Start();
		OnResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
		// https://learn.microsoft.com/zh-cn/windows/win32/winmsg/wm-destroy
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
		// https://learn.microsoft.com/zh-cn/windows/win32/menurc/wm-menuchar
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
		// https://learn.microsoft.com/zh-cn/windows/win32/winmsg/wm-getminmaxinfo
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		return 0;
	case WM_MOUSEMOVE:
		return 0;
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = ::MainWndProc;
	wc.lpszClassName = L"D3DWndClassName";
	wc.hCursor = LoadCursorW(0, IDC_ARROW);
	wc.hInstance = m_hAppInst;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hIcon = LoadIconW(0, IDI_APPLICATION);
	wc.lpszMenuName = nullptr;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassW(&wc))
	{
		MessageBoxW(0, L"RegisterClassW Failed.", 0, 0);
		return false;
	}

	RECT R = { 0, 0, m_ClientWidth, m_ClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_hMainWnd = CreateWindowW(L"D3DWndClassName", m_MainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_hAppInst, 0);
	if (!m_hMainWnd)
	{
		MessageBoxW(0, L"CreateWindowW Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	return true;
}

bool D3DApp::InitDirect3D()
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = { 0 };
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// 驱动类型数组
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = _ARRAYSIZE(driverTypes);

	// 特性等级数组
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	UINT numFeatureLevels = _ARRAYSIZE(featureLevels);

	D3D_FEATURE_LEVEL featureLevel;
	D3D_DRIVER_TYPE driverType;
	// 创建 D3D设备 和 设备上下文
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, driverType, 0, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, m_pd3dDevice.GetAddressOf(), &featureLevel, m_pd3dImmediateContext.GetAddressOf());
		if (hr == E_INVALIDARG)
		{
			// Direct3D 11.0 的 API 不支持 D3D_FEATURE_LEVEL_11_1，所以我们需要尝试特性等级 11.0 以及以下的版本
			hr = D3D11CreateDevice(nullptr, driverType, 0, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, m_pd3dDevice.GetAddressOf(), &featureLevel, m_pd3dImmediateContext.GetAddressOf());
		}

		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
	{
		MessageBoxW(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	// 检测是否支持特性等级 11.0 或 11.1
	if (featureLevel != D3D_FEATURE_LEVEL_11_1 && featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBoxW(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	// 检查 MSAA 支持的质量等级
	m_pd3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality);
	assert(m_4xMsaaQuality > 0);




	ComPtr<IDXGIDevice>		dxgiDevice = nullptr;
	ComPtr<IDXGIAdapter>	dxgiAdapter = nullptr;
	ComPtr<IDXGIFactory1>	dxgiFactory1 = nullptr;  // D3D11.0 (包含DXGI1.1) 的接口类
	ComPtr<IDXGIFactory2>	dxgiFactory2 = nullptr;  // D3D11.1 (包含DXGI1.2) 的接口类

	// 为了正确创建 DXGI交换链，首先我们需要获取创建 D3D设备 的 DXGI工厂，否则会引发报错：
	// "IDXGIFactory::CreateSwapChain: This function is being called with a device from a different IDXGIFactory."
	HR(m_pd3dDevice.As(&dxgiDevice));
	HR(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory1.GetAddressOf())));

	// 查看该对象是否包含 IDXGIFactory2 接口
	hr = dxgiFactory1.As(&dxgiFactory2);
	// 如果包含，则说明支持 D3D11.1
	if (dxgiFactory2 != nullptr)
	{
		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		sd.BufferCount = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.Flags = 0;
		sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.Width = m_ClientWidth;
		sd.Height = m_ClientHeight;
		// 是否开启 4倍多重采样？
		if (m_bEnable4xMsaa)
		{
			sd.SampleDesc.Count = 4;
			sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
		}
		else
		{
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
		}
		sd.Scaling = DXGI_SCALING_STRETCH;
		sd.Stereo = FALSE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd;
		ZeroMemory(&fd, sizeof(fd));
		fd.RefreshRate.Numerator = 60;
		fd.RefreshRate.Denominator = 1;
		fd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fd.Windowed = TRUE;

		// 为当前窗口创建交换链
		HR(dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice.Get(), m_hMainWnd, &sd, &fd, nullptr, m_pSwapChain1.GetAddressOf()));
		HR(m_pSwapChain1.As(&m_pSwapChain));
	}
	else
	{
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = m_ClientWidth;
		sd.BufferDesc.Height = m_ClientHeight;
		sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.Windowed = TRUE;
		// 是否开启 4倍多重采样？
		if (m_bEnable4xMsaa)
		{
			sd.SampleDesc.Count = 4;
			sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
		}
		else
		{
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
		}
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.Flags = 0;
		sd.OutputWindow = m_hMainWnd;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		// 为当前窗口创建交换链
		HR(dxgiFactory1->CreateSwapChain(m_pd3dDevice.Get(), &sd, m_pSwapChain.GetAddressOf()));
	}



	// 可以禁止 alt+enter 全屏
	dxgiFactory1->MakeWindowAssociation(m_hMainWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pd3dImmediateContext.Get(), "ImmediateContext");
	DXGISetDebugObjectName(m_pSwapChain.Get(), "SwapChain");

	// 每当窗口被重新调整大小的时候，都需要调用这个 OnResize 函数
	// 现在调用以避免代码重复
	OnResize();

	return true;
}

void D3DApp::CalculateFrameStats()
{
	static int frameCount = 0;
	static float timeElapsed = 0.0f;

	frameCount++;

	if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = static_cast<float>(frameCount);
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << m_MainWndCaption << L"    "
			<< "Fps: " << fps << L"    "
			<< "Frame Time: " << mspf << L" (ms)";
		SetWindowTextW(m_hMainWnd, outs.str().c_str());

		// Reset for next average.
		frameCount = 0;
		timeElapsed += 1.0f;
	}
}
