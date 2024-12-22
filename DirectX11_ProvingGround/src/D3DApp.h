#ifndef D3DAPP_H
#define D3DAPP_H

#include <wrl/client.h>
#include <string>
#include <DirectXMath.h>
#include <d3d11_1.h>
#include "WinMin.h"
#include "CpuTimer.h"


class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
	virtual ~D3DApp();

public:
	int Run();


	/************************************************************/
	/** 框架方法。客户派生类需要从重载这些方法以实现特定的应用需求 */
	/************************************************************/

	/**
	* @comment 该父类方法需要初始化窗口和 Direct3D 部分
	*/
	virtual bool Init();

	/**
	* @comment 该父类方法需要在窗口大小变动的时候调用
	*/
	virtual void OnResize();

	/**
	* @comment 子类需要实现该方法，完成每一帧的更新
	*/
	virtual void UpdateScene(float deltaTime) = 0;

	/**
	* @comment 子类需要实现该方法，完成每一帧的绘制
	*/
	virtual void DrawScene() = 0;

	/**
	* @comment 窗口的消息回调函数
	*/
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	/**
	* @comment 初始化主窗口
	*/
	bool InitMainWindow();

	/**
	* @comment 初始化 Direct3D
	*/
	bool InitDirect3D();

	/**
	* @comment 计算每秒帧数并在窗口显示
	*/
	void CalculateFrameStats();

public:
	HINSTANCE AppInSt() const { return m_hAppInst; }
	HWND MainWnd() const { return m_hMainWnd; }
	float AspectRadio() const { return static_cast<float>(m_ClientWidth) / m_ClientHeight; }

protected:
	HINSTANCE	m_hAppInst;			// 应用实例句柄
	HWND		m_hMainWnd;			// 主窗口句柄
	bool		m_bAppPaused;		// 应用是否暂停
	bool		m_bMinimized;		// 应用是否最小化
	bool		m_bMaximized;		// 应用是否最大化
	bool		m_bResizing;		// 窗口大小是否变化
	bool		m_bEnable4xMsaa;	// 是否开启 4x 多重采样
	UINT		m_4xMsaaQuality;	// 4x 多重采样质量等级

	// Cpu 计时器
	CpuTimer m_Timer;

	// 使用模板别名（C++11）简化类型名
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	// Direct3D 11
	ComPtr<ID3D11Device>			m_pd3dDevice;				// D3D11 设备
	ComPtr<ID3D11DeviceContext>		m_pd3dImmediateContext;		// D3D11 设备上下文
	ComPtr<IDXGISwapChain>			m_pSwapChain;				// D3D11 交换链
	// Direct3D 11.1
	ComPtr<ID3D11Device1>			m_pd3dDevice1;				// D3D11.1 设备
	ComPtr<ID3D11DeviceContext1>	m_pd3dImmediateContext1;	// D3D11.1 设备上下文
	ComPtr<IDXGISwapChain1>			m_pSwapChain1;				// D3D11.1 交换链
	// 常用资源
	ComPtr<ID3D11Texture2D>			m_pDepthStencilBuffer;		// 深度模板缓冲
	ComPtr<ID3D11RenderTargetView>	m_pRenderTargetView;		// 渲染目标视图
	ComPtr<ID3D11DepthStencilView>	m_pDepthStencilView;		// 深度模板视图
	D3D11_VIEWPORT					m_ScreenViewport;			// 视口

protected:
	std::wstring	m_MainWndCaption;	// 主窗口标题
	int				m_ClientWidth;		// 视口宽度
	int				m_ClientHeight;		// 视口高度
};

#endif  // D3DAPP_H