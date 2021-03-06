// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
#include <windows.h>
#include <windowsx.h>


// TODO: 在此处引用程序需要的其他头文件
#include <strsafe.h>

inline LPCTSTR GetLastErrorInfo(LPCTSTR lpInfo = TEXT(""))
{
	static TCHAR szError[MAX_PATH];
	LPCTSTR lpBuf = NULL;
	FormatMessage(0x00001100, 0, GetLastError(), MAKELANGID(0, 1), (LPTSTR)&lpBuf, 0, NULL);
	if (lpInfo[0] != '\0')
	{
		if (lpBuf == NULL)
			StringCchPrintf(szError, MAX_PATH, TEXT("%s:获取错误信息失败！"), lpInfo);
		else
			StringCchPrintf(szError, MAX_PATH, TEXT("%s:%s"), lpInfo, lpBuf);
		return szError;
	}
	else return lpBuf ? lpBuf : lpInfo;
}
