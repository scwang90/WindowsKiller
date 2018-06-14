#pragma once

#include "IWndEngine.h"

#include <string>
typedef std::basic_string<TCHAR> tstring;

class WndEngine :public IWndEngine
{
public:
	WndEngine(void);
public:
	~WndEngine(void);

public:
	// IUnknown member function
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void **ppv);
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

	// IWndEngine member function	
	virtual	bool	IsValidWindow();
	virtual	bool	TurnToParent();
	virtual	bool	UpDateWndInfo();
	virtual	bool	QueryWindow(HWND hwnd);
	virtual	bool	QueryProcess(DWORD dwPid);
	virtual	void	SendMessage(LPCTSTR lpText);
	virtual	void	SetWindowText(LPCTSTR lpText);
	virtual	bool	SetWinRect(LPRECT lpRect);
	virtual	bool	FindWindow(LPPOINT lpPoint, UINT flags = 0xFFFFFFFF);
	virtual	HWND	FindWindowEx(HWND hPWnd, HWND hAfter, LPCTSTR lpClass, LPCTSTR lpWnd, bool bCaseMatters = false, bool bIsFuzzy = false);
	virtual	LPCTSTR GetStageInfo(int iInfo);
	virtual	bool	SendFunction(int iFun);
	virtual	HWND	GetWindow(int iWnd = 0);
	virtual	LPRECT	GetWndRect(int iRect = 0);
	virtual	bool	GetPriorityClass(DWORD dwPid, LPDWORD lpPriorityClass);
	virtual	bool	SetPriorityClass(DWORD dwPid, DWORD dwPriorityClass);

private:
	void UpDateInfo();
	bool UpPrivilege();
	void GetWindowClass();
	void GetWndStyleInfo();
	void GetWndParentInfo();
	bool GetWndModuleInfo();
	bool GetProcessModules();
	void EnumWindowChlid(HWND hWnd, int state = 0);

private:
	static BOOL CALLBACK CheckChildWnd(HWND hwnd, LPARAM lparam);

private:
	tstring m_szWindows;
	tstring	m_szInfo[all];

private:
	// IUnknown member data	
	int		m_Ref;

	// IWndEngine member function

	HWND m_hCWnd;
	HWND m_hPWnd;
	HWND m_hDWnd;
	RECT m_CurWinRect[RECT_ALL];

	//LPCW m_pChildWnd;

	DWORD m_dwhWndTid;
	DWORD m_dwProcPid;
	DWORD m_dwProcModPrePid;
};
