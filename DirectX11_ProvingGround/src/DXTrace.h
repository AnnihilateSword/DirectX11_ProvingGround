#ifndef DXTRACE_H
#define DXTRACE_H

#include <Windows.h>


/**
* @comment 在调试输出窗口中输出格式化错误信息，可选的错误窗口弹出（已汉化）
* 
* @param strFile		当前文件名，通常传递宏 __FILEW__
* @param dwLine			当前行号，通常传递宏 __LINE__
* @param hr				函数执行出现问题时返回的 HRESULT
* @param strMsg			用于帮助调试定位的字符串，通常传递 L#x（可能为 NULL）
* @param bPopMsgBox		如果为 TRUE，则弹出一个消息弹窗告知错误信息
* 
* @return 形参 hr
*/
HRESULT WINAPI DXTraceW(const WCHAR* strFile, DWORD dwLine, HRESULT hr, const WCHAR* strMsg, bool bPopMsgBox);


// Debug 模式下的错误提醒与追踪
#if defined(DEBUG) || defined(_DEBUG)
	#ifndef HR
	#define HR(x)													\
	{																\
		HRESULT hr = (x);											\
		if (FAILED(hr))												\
		{															\
			DXTraceW(__FILEW__, (DWORD)__LINE__, hr, L#x, true);	\
		}															\
	}
	#endif
#else
	#ifndef HR(x)
	#define HR(x) (x)
	#endif
#endif




#endif  // DXTRACE_H