#ifndef D3DUTIL_H
#define D3DUTIL_H

#include <d3d11_1.h>
#include <string>


// 默认开启图像调试器具名化
// 如果不需要该功能，可通过全局文本替换将其值设置为 0
#ifndef GRAPHICS_DEBUGGER_OBJECT_NAME
#define GRAPHICS_DEBUGGER_OBJECT_NAME (1)
#endif


// 安全 COM  组件释放宏
#define SAFE_RELEASE(p) if ((p)) { (p)->Release(); (p) = nullptr; }




/**
* @comment 为 D3D设备 创建出来的对象在 图形调试器中设置对象名
* 
* @param resource	D3D11设备创建出来的对象
* @param name		对象名
*/
template<UINT TNameLength>
inline void D3D11SetDebugObjectName(ID3D11DeviceChild* resource, const char(&name)[TNameLength])
{
#if defined(DEBUG) || defined(_DEBUG)
	resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
#else
	UNREFERENCED_PARAMETER(resource);
	UNREFERENCED_PARAMETER(name);
#endif
}


/**
* @comment 为 D3D设备 创建出来的对象在 图形调试器中设置对象名
*
* @param resource	D3D11设备创建出来的对象
* @param name		对象名
* @param length		字符串长度
*/
inline void D3D11SetDebugObjectName(ID3D11DeviceChild* resource, LPCSTR name, UINT length)
{
#if defined(DEBUG) || defined(_DEBUG)
	resource->SetPrivateData(WKPDID_D3DDebugObjectName, length, name);
#else
	UNREFERENCED_PARAMETER(resource);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(length);
#endif
}


/**
* @comment 为 D3D设备 创建出来的对象在 图形调试器中设置对象名
*
* @param resource	D3D11设备创建出来的对象
* @param name		对象名
*/
inline void D3D11SetDebugObjectName(ID3D11DeviceChild* resource, std::string& name)
{
#if defined(DEBUG) || defined(_DEBUG)
	resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.length()), name.c_str());
#else
	UNREFERENCED_PARAMETER(resource);
	UNREFERENCED_PARAMETER(name);
#endif
}


/**
* @comment 为 D3D设备 创建出来的对象在 图形调试器中设置对象名
*
* @param resource	D3D11设备创建出来的对象
*/
inline void D3D11SetDebugObjectName(ID3D11DeviceChild* resource, std::nullptr_t)
{
#if defined(DEBUG) || defined(_DEBUG)
	resource->SetPrivateData(WKPDID_D3DDebugObjectName, 0, nullptr);
#else
	UNREFERENCED_PARAMETER(resource);
#endif
}


/**
* @comment 为 DXGI 对象在 图形调试器中设置对象名
*
* @param object		DXGI 对象
* @param name		对象名
*/
template<UINT TNameLength>
inline void DXGISetDebugObjectName(IDXGIObject* object, const char(&name)[TNameLength])
{
#if defined(DEBUG) || defined(_DEBUG)
	object->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
#else
	UNREFERENCED_PARAMETER(resource);
	UNREFERENCED_PARAMETER(name);
#endif
}


/**
* @comment 为 DXGI 对象在 图形调试器中设置对象名
*
* @param object		DXGI 对象
* @param name		对象名
* @param length		字符串长度
*/
inline void DXGISetDebugObjectName(IDXGIObject* object, LPCSTR name, UINT length)
{
#if defined(DEBUG) || defined(_DEBUG)
	object->SetPrivateData(WKPDID_D3DDebugObjectName, length, name);
#else
	UNREFERENCED_PARAMETER(object);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(length);
#endif
}


/**
* @comment 为 DXGI 对象在 图形调试器中设置对象名
*
* @param object		DXGI 对象
* @param name		对象名
*/
inline void DXGISetDebugObjectName(IDXGIObject* object, std::string& name)
{
#if defined(DEBUG) || defined(_DEBUG)
	object->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.length()), name.c_str());
#else
	UNREFERENCED_PARAMETER(object);
	UNREFERENCED_PARAMETER(name);
#endif
}


/**
* @comment 为 DXGI 对象在 图形调试器中设置对象名
*
* @param object		DXGI 对象
*/
inline void DXGISetDebugObjectName(IDXGIObject* object, std::nullptr_t)
{
#if defined(DEBUG) || defined(_DEBUG)
	object->SetPrivateData(WKPDID_D3DDebugObjectName, 0, nullptr);
#else
	UNREFERENCED_PARAMETER(object);
#endif
}

#endif  // D3DUTIL_H