
// WindowsKiller3.0.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CWindowsKiller30App:
// 有关此类的实现，请参阅 WindowsKiller3.0.cpp
//

#define	WM_RESTARTKILLER	(WM_USER+128)

class CWindowsKiller30App : public CWinApp
{
public:
	CWindowsKiller30App();

// 重写
public:
	virtual BOOL InitInstance();

public:
	static	HRESULT  DllCoCreateObject(const CLSID& clsid, const IID& iid, void **ppv);
// 实现

	DECLARE_MESSAGE_MAP()


};

extern CWindowsKiller30App theApp;
