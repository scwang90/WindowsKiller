// stdafx.cpp : 只包括标准包含文件的源文件
// Common.pch 将作为预编译标头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中引用任何所需的附加头文件，
//而不是在此文件中引用
#include <strsafe.h>

LPCTSTR COMMON_API GetLastErrorInfo(LPCTSTR lpInfo)
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
