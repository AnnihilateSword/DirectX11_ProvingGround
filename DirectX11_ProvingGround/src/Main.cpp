#include "GameApp.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")


int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	// 这些参数不使用
	UNREFERENCED_PARAMETER(hInstPrev);
	UNREFERENCED_PARAMETER(cmdline);
	UNREFERENCED_PARAMETER(cmdshow);

	// 允许在 Debug 版本进行运行时内存分配和泄漏检测
	// https://learn.microsoft.com/zh-cn/cpp/c-runtime-library/reference/crtsetdbgflag?view=msvc-170
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	GameApp app(hInst, L"DirectX11 Initialization", 1280, 720);

	if (!app.Init())
		return 0;

	return app.Run();
}