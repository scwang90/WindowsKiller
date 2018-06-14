#pragma once

#include <Unknwn.h>

// {09980499-7D26-4260-8779-4ACDDA9B8E9C}
extern "C" const GUID IID_IWndEngine;

enum EWNDINFO {
	WNDINFO_HANDLE,
	WNDINFO_TIELE,
	WNDINFO_CLASS,
	WNDINFO_PROCESSID,
	WNDINFO_PROCESSNAME,
	WNDINFO_MODULENAME,
	WNDINFO_PROCESSPATH,
	WNDINFO_MODULEPATH,
	WNDINFO_ALLINFO
};


interface IWndEngine : public IUnknown
{
	virtual	bool	IsValidWindow() = 0;
	virtual	bool	TurnToParent() = 0;
	virtual	bool	UpDateWndInfo() = 0;
	virtual	bool	QueryWindow(HWND hwnd) = 0;
	virtual	bool	QueryProcess(DWORD dwPid) = 0;
	virtual	void	SendMessage(LPCTSTR lpText) = 0;
	virtual	void	SetWindowText(LPCTSTR lpText) = 0;
	virtual	bool	SetWinRect(LPRECT lpRect) = 0;
	virtual	bool	FindWindow(LPPOINT lpPoint, UINT flags = 0xFFFFFFFF) = 0;
	virtual	HWND	FindWindowEx(HWND hPWnd, HWND hAfter, LPCTSTR lpClass, LPCTSTR lpWnd, bool bCaseMatters = false, bool bIsFuzzy = false) = 0;
	virtual	LPCTSTR GetStageInfo(int iInfo) = 0;
	virtual	bool	SendFunction(int iFun) = 0;
	virtual	HWND	GetWindow(int iWnd = 0) = 0;
	virtual	LPRECT	GetWndRect(int iRect = 0) = 0;
	virtual	bool	GetPriorityClass(DWORD dwPid, LPDWORD lpPriorityClass) = 0;
	virtual	bool	SetPriorityClass(DWORD dwPid, DWORD dwPriorityClass) = 0;

	enum Rect {
		RECT_WINDOW, RECT_CLIENT, RECT_RESTORE,
		RECT_ALL
	};
	enum GWnd {
		GETWINDOW_CURWND, GETWINDOW_NEXT, GETWINDOW_PREV,
		GETWINDOW_PARENT, GETWINDOW_FIRSTCHILD, GETWINDOW_OWNER
	};
	enum Info {
		WND_HANDLE, WND_TEXT, WND_CLSNAME, WND_CLSVALUE,
		WND_STYLE, WND_EXSTYLE, WND_PHANDLE, WND_PTEXT, WND_PCLSNAME,
		WND_PROCESSID, PROC_PROCESSID = WND_PROCESSID, PROC_THREADID,
		WND_MODULENAME, WND_MODULEPATH, WND_PROCESSNAME,
		WND_PROCESSPATH, WND_PEOCESSHINST, WND_MODULEHINST, WND_MENU,

		CLS_NAME, CLS_STYLE, CLS_BYTE, CLS_ATOM,
		CLS_WNDBYTE, CLS_MEUN, CLS_ICON, CLS_CURSOR,
		CLS_HBRUSH, CLS_INSTANCE, CLS_WINPROCESS,

		PRO_MODULES,

		LAST_ERROR,
		all
	};
	enum Function {
		FUN_MAXSIZE, FUN_MINSIZE, FUN_NORMAL, FUN_DISABLE, FUN_ENABLE,
		FUN_HIDE, FUN_SHOW, FUN_TOPMOST, FUN_DISTOPMOST,
		FUN_CLOSE, FUN_DESTROY, FUN_KILLPROCESS
	};
	enum PriorityClass {
		////////////////////////////////////////////////////////////////////////////////////	
		//arg for SetPriorityClass

		PRIORITY_CLASS_REALTIME = 0x00000100,
		PRIORITY_CLASS_HIGH = 0x00000080,
		PRIORITY_CLASS_NORMAL_ABOVE = 0x00008000,
		PRIORITY_CLASS_NORMAL = 0x00000020,
		PRIORITY_CLASS_NORMAL_BELOW = 0x00004000,
		PRIORITY_CLASS_IDLE = 0x00000040,

#ifndef PROCESS_MODE_BACKGROUND_BEGIN
		PROCESS_MODE_BACKGROUND_BEGIN = 0x00100000,
		//Windows Server 2003 and Windows XP/2000:  This value is not supported. 
#endif
#ifndef PROCESS_MODE_BACKGROUND_END
		PROCESS_MODE_BACKGROUND_END = 0x00200000
		//This value can be specified only if hProcess is a handle to the current process.
		//The function fails if the process is not in background processing mode.
		//Windows Server 2003 and Windows XP/2000:  This value is not supported. 
#endif
		////////////////////////////////////////////////////////////////////////////////////////

	};
};

#ifdef WNDENGINE_EXPORTS
#define WNDENGINE_API __declspec(dllexport)
#else
#define WNDENGINE_API __declspec(dllimport)
#endif

class WNDENGINE_API DllWndEngine{
public:
	// TODO: 在此添加您的方法。
	static	HRESULT  DllCoCreateObject(const CLSID& clsid, const IID& iid, void **ppv);
};

// {0387EC96-5C4D-44b0-B26F-7B32AF292285}
extern "C" const GUID CLSID_IWndEngine;
