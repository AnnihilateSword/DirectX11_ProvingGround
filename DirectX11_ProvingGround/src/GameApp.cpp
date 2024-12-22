#include "GameApp.h"

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
	: D3DApp(hInstance, windowName, initWidth, initHeight)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!InitMainWindow())
		return false;

	if (!InitDirect3D())
		return false;

	return true;
}

void GameApp::OnResize()
{
	D3DApp::OnResize();
}

void GameApp::UpdateScene(float deltaTime)
{
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);
	static float red[4] = { 0.8f, 0.2f, 0.2f, 1.0f };
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), red);
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// https://learn.microsoft.com/zh-cn/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-present
	m_pSwapChain->Present(0, 0);
}
