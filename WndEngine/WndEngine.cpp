// WndEngine.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "WndEngine.h"

#include <Psapi.h>
#include <tlhelp32.h>
#include <windowsx.h>
#include <tchar.h>

// class IWndEngine implementation

ULONG    g_LockNumber = 0;
ULONG    g_IWndEngineNumber = 0;
HANDLE	 g_hModule;

WndEngine::WndEngine(void)
{
	//////////////////////////////////////////////////////////////////////////////
	//	Unknown
	m_Ref = 0;
	g_IWndEngineNumber++;
	//////////////////////////////////////////////////////////////////////////////
	//	IWndEngine
	m_hCWnd = NULL;
	m_hPWnd = NULL;
	m_dwProcPid = 0;
	m_dwProcModPrePid = 0;

	m_hDWnd = ::GetDesktopWindow();

	//m_pChildWnd = NULL;

	if (g_IWndEngineNumber == 1)UpPrivilege();
	this->m_szInfo[LAST_ERROR] = TEXT("操作未执行。");
}

WndEngine::~WndEngine(void)
{
	//if(m_pChildWnd != NULL)
	//{
	//	WndEngine::ClearLPCW(m_pChildWnd);
	//}
}

HRESULT  DllWndEngine::DllCoCreateObject(const CLSID& clsid, const IID& iid, void **ppv)
{
	if (clsid == CLSID_IWndEngine) {

		WndEngine *pFactory = new WndEngine;

		if (pFactory == NULL) {
			return E_OUTOFMEMORY;
		}

		HRESULT result = pFactory->QueryInterface(iid, ppv);

		return result;
	}
	else {
		return CLASS_E_CLASSNOTAVAILABLE;
	}
}

HRESULT  WndEngine::QueryInterface(const IID& iid, void **ppv)
{
	if (iid == IID_IUnknown)
	{
		*ppv = (IUnknown *)this;
		((IUnknown *)(*ppv))->AddRef();
	}
	else if (iid == IID_IWndEngine)
	{
		*ppv = (IWndEngine *)this;
		((IWndEngine *)(*ppv))->AddRef();
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG	 WndEngine::AddRef()
{
	m_Ref++;
	return  (ULONG)m_Ref;
}

ULONG	 WndEngine::Release()
{
	m_Ref--;
	if (m_Ref == 0) {
		g_IWndEngineNumber--;
		delete this;
		return 0;
	}
	return  (ULONG)m_Ref;
}
bool	WndEngine::FindWindow(LPPOINT lpPoint, UINT flags)
{
	static	DWORD		dwPid;
	static	HWND		hWnd, hCwnd;

	if (-1 == flags)
	{
		hWnd = WindowFromPoint(*lpPoint);
	}
	else
	{
		for (hWnd = m_hDWnd;;)
		{
			::ScreenToClient(hWnd, lpPoint);
			hCwnd = ChildWindowFromPointEx(hWnd, *lpPoint, flags);
			::ClientToScreen(hWnd, lpPoint);
			if (hCwnd != hWnd)hWnd = hCwnd;
			else break;
		}
		if (hWnd == nullptr) {
			hWnd = WindowFromPoint(*lpPoint);
		}
	}

	GetWindowThreadProcessId(hWnd, &dwPid);
	if (hWnd && (hWnd != m_hCWnd) && dwPid != GetCurrentProcessId())
	{
		this->m_hCWnd = hWnd;
		this->UpDateInfo();
		::GetWindowRect(m_hCWnd, &m_CurWinRect[RECT_RESTORE]);
		this->m_szInfo[LAST_ERROR] = TEXT("操作成功执行。");
		return TRUE;
	}
	this->m_szInfo[LAST_ERROR] = TEXT("窗口未找到或窗口重复。");
	return FALSE;
}

HWND	WndEngine::GetWindow(int wnd)
{
	switch (wnd)
	{
	case GETWINDOW_CURWND:
		return m_hCWnd;
	case GETWINDOW_NEXT:
		return ::GetNextWindow(m_hCWnd, GW_HWNDNEXT);
	case GETWINDOW_PREV:
		return ::GetNextWindow(m_hCWnd, GW_HWNDPREV);
	case GETWINDOW_PARENT:
		return ::GetParent(m_hCWnd);
	case GETWINDOW_FIRSTCHILD:
		return ::GetWindow(m_hCWnd, GW_CHILD);
	case GETWINDOW_OWNER:
		return ::GetWindow(m_hCWnd, GW_OWNER);
	}
	return NULL;
}

LPRECT	WndEngine::GetWndRect(int iRect)
{
	return &m_CurWinRect[iRect];
}

bool	WndEngine::SetWinRect(LPRECT lpRect)
{
	if (lpRect->right < lpRect->left
		|| lpRect->bottom < lpRect->top)
	{
		this->m_szInfo[LAST_ERROR] = TEXT("矩形参数错误。");
		return false;
	}
	else if (FALSE == IsWindow(m_hCWnd))
	{
		this->m_szInfo[LAST_ERROR] = TEXT("窗口不存在。");
		return false;
	}

	if (m_hPWnd != NULL)
	{
		lpRect->right -= lpRect->left;
		lpRect->bottom -= lpRect->top;
		ScreenToClient(m_hPWnd, LPPOINT(lpRect));
		lpRect->right += lpRect->left;
		lpRect->bottom += lpRect->top;
	}

	m_CurWinRect[0] = *lpRect;

	RECT& crect = m_CurWinRect[0];

	MoveWindow(m_hCWnd, crect.left, crect.top, crect.right - crect.left, crect.bottom - crect.top, TRUE);

	this->m_szInfo[LAST_ERROR] = TEXT("操作成功执行。");
	return true;
}

LPCTSTR WndEngine::GetStageInfo(int info)
{
	if (info > all - 1 || info < 0)
	{
		this->m_szInfo[LAST_ERROR] = TEXT("不可知的标记。");
		return NULL;
	}
	else if (info == PRO_MODULES)
	{
		this->GetProcessModules();
	}
	return m_szInfo[info].c_str();
}

void WndEngine::SendMessage(LPCTSTR lpText)
{
	DWORD_PTR dwRet = 0;
	for (LPCTSTR lpstr = lpText; *lpstr; lpstr++)
	{
		if (*lpstr == '\\')
		{
			if (lpstr[1] == 't')
			{
				//FORWARD_WM_KEYDOWN(m_hCWnd,VK_TAB,0,0,::SendMessage);
				SendMessageTimeout(m_hCWnd, WM_KEYDOWN, WPARAM(VK_TAB), MAKELPARAM(0, 0), SMTO_NORMAL, 100, &dwRet);
				lpstr++;
				continue;
			}
			if (lpstr[1] == 'n')
			{
				//FORWARD_WM_KEYDOWN(m_hCWnd,VK_RETURN,0,0,::SendMessage);
				SendMessageTimeout(m_hCWnd, WM_KEYDOWN, WPARAM(VK_RETURN), MAKELPARAM(0, 0), SMTO_NORMAL, 100, &dwRet);
				lpstr++;
				continue;
			}
		}
		SendMessageTimeout(m_hCWnd, WM_CHAR, WPARAM(*lpstr), MAKELPARAM(0, 0), SMTO_NORMAL, 100, &dwRet);
		//FORWARD_WM_CHAR(m_hCWnd,*lpstr,0,::SendMessage);
	}
}

void WndEngine::SetWindowText(LPCTSTR lpText)
{
	//FORWARD_WM_SETTEXT(m_hCWnd,lpText,::SendMessage);
	DWORD_PTR dwRet = 0;
	SendMessageTimeout(m_hCWnd, WM_SETTEXT, 0L, LPARAM(lpText), SMTO_NORMAL, 100, &dwRet);

}

bool WndEngine::TurnToParent()
{
	if (FALSE == IsWindow(m_hCWnd))
	{
		this->m_szInfo[LAST_ERROR] = TEXT("窗口不存在。");
		return false;
	}

	if (m_hPWnd == m_hCWnd)
	{
		this->m_szInfo[LAST_ERROR] = TEXT("父窗口不存在。");
		return false;
	}

	m_hCWnd = m_hPWnd;
	this->UpDateInfo();
	this->m_szInfo[LAST_ERROR] = TEXT("操作成功执行。");
	return true;
}

bool WndEngine::UpDateWndInfo()
{
	if (IsWindow(m_hCWnd))
	{
		UpDateInfo();
		this->m_szInfo[LAST_ERROR] = TEXT("操作成功执行。");
		return true;
	}
	this->m_szInfo[LAST_ERROR] = TEXT("窗口不存在。");
	return false;
}

bool WndEngine::SendFunction(int fun)
{
	try {
		if (FALSE == IsWindow(m_hCWnd) && FUN_KILLPROCESS != fun)
			throw TEXT("窗口不存在。");

		switch (fun)
		{
		case FUN_MAXSIZE:
			ShowWindow(m_hCWnd, SW_MAXIMIZE);
			break;
		case FUN_MINSIZE:
			ShowWindow(m_hCWnd, SW_MINIMIZE);
			break;
		case FUN_NORMAL:
			ShowWindow(m_hCWnd, SW_NORMAL);
			break;
		case FUN_DISABLE:
			if (EnableWindow(m_hCWnd, FALSE) != FALSE)
				throw ::GetLastErrorInfo();
			break;
		case FUN_ENABLE:
			if (EnableWindow(m_hCWnd, TRUE) == FALSE)
				throw ::GetLastErrorInfo();
			break;
		case FUN_HIDE:
			if (::IsWindowVisible(m_hCWnd) == FALSE)break;
			if (ShowWindow(m_hCWnd, SW_HIDE) == FALSE)
				throw ::GetLastErrorInfo();
			break;
		case FUN_SHOW:
			if (::IsWindowVisible(m_hCWnd) == TRUE)break;
			if (ShowWindow(m_hCWnd, SW_SHOW) != FALSE)
				throw ::GetLastErrorInfo();
			break;
		case FUN_TOPMOST:
		{
			DWORD_PTR	dwStyle = GetWindowExStyle(m_hCWnd);
			if ((dwStyle & WS_EX_TOPMOST) != 0)break;
		}
		if (SetWindowPos(m_hCWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE) == FALSE)
			throw ::GetLastErrorInfo();
		break;
		case FUN_DISTOPMOST:
		{
			DWORD_PTR	dwStyle = GetWindowExStyle(m_hCWnd);
			if ((dwStyle & WS_EX_TOPMOST) == 0)break;
		}
		if (SetWindowPos(m_hCWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE) == FALSE)
			throw ::GetLastErrorInfo();
		break;
		case FUN_CLOSE:
		{
			//FORWARD_WM_SYSCOMMAND(m_hCWnd,SC_CLOSE,0,0,::SendMessage);
			DWORD_PTR	dwRet = 0;
			SendMessageTimeout(m_hCWnd, WM_SYSCOMMAND, WPARAM(SC_CLOSE), MAKELPARAM(0, 0), SMTO_NORMAL, 100, &dwRet);

		}
		break;
		case FUN_DESTROY:
		{
			//			CString	szDllPath = TEXT("d:\\我的文档\\C-Free\\Projects\\TestDll\\VC\\TestDll.dll");
			//	
			//
			/*			DWORD_PTR	dwRemoteProcessId;
			GetWindowThreadProcessId(m_hCWnd,&dwRemoteProcessId);
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwRemoteProcessId);  */
			//
			//#ifdef _DEBUG
			//			if(hProcess == NULL)throw ::GetLastErrorInfo();
			//#endif
			//			
			//			HWND* pHwnd = (HWND*)VirtualAllocEx(hProcess,NULL,/*1+szDllPath.GetLength()*/sizeof(HWND),MEM_COMMIT,PAGE_READWRITE);  
			//			
			//#ifdef _DEBUG
			//			if(pHwnd == NULL)throw ::GetLastErrorInfo();
			//#endif
			//			//将DLL的路径名写入到远程进程的内存空间
			//			WriteProcessMemory(hProcess,pHwnd,&m_hCWnd/*(void *)LPCTSTR(szDllPath)*/,/*1+szDllPath.GetLength()*/sizeof(HWND), NULL);  
			//
			//			HANDLE hThread = CreateRemoteThread(hProcess,NULL,0,(LPTHREAD_START_ROUTINE)DestroyWindow,pHwnd,0,&dwRemoteProcessId);
			//
			//#ifdef _DEBUG
			//			if(hThread == NULL)throw ::GetLastErrorInfo();
			//#endif
			//			if (hThread)
			//			{
			//				DWORD_PTR h;
			//				::WaitForSingleObject( hThread, INFINITE );
			//				::GetExitCodeThread( hThread, &h );
			//				printf("run and return %d\n",h);
			//			}
			//			CloseHandle(hThread);
			//
			//			AfxMessageBox("LoadLibrary");
			//
			//			hThread = CreateRemoteThread(hProcess,NULL,0,(LPTHREAD_START_ROUTINE)FreeLibrary,pHwnd,0,&dwRemoteProcessId);
			//
			//#ifdef _DEBUG
			//			if(hThread == NULL)throw ::GetLastErrorInfo();
			//#endif
			//			if (hThread)
			//			{
			//				DWORD_PTR h;
			//				::WaitForSingleObject( hThread, INFINITE );
			//				::GetExitCodeThread( hThread, &h );
			//				TRACE("run and return %d\n",h);
			//				printf("run and return %d\n",h);
			//			}
			//			CloseHandle(hThread); 
			//
			//			VirtualFreeEx( hProcess, pHwnd,1+szDllPath.GetLength()/*sizeof(HWND)*/,MEM_RELEASE );
			//
			//			CloseHandle(hProcess); 
			//FORWARD_WM_DESTROY(m_hCWnd,::SendMessage);		
			DWORD_PTR	dwRet = 0;
			SendMessageTimeout(m_hCWnd, WM_DESTROY, WPARAM(0), LPARAM(0), SMTO_NORMAL, 100, &dwRet);


			//return TRUE;  
			//if(DestroyWindow(m_hCWnd) == FALSE)
			//	throw ::GetLastErrorInfo();
		}
		break;
		case FUN_KILLPROCESS:
		{
			//DWORD_PTR PID[1] = {0};
			//GetWindowThreadProcessId (m_hCWnd,PID);
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, m_dwProcPid);
			if (hProcess != NULL)
			{
				LPCTSTR	lpError = NULL;
				if (TerminateProcess(hProcess, 0) == FALSE)
					lpError = GetLastErrorInfo();
				CloseHandle(hProcess);

				if (lpError != NULL)throw lpError;
			}
			else throw ::GetLastErrorInfo();
		}
		break;
		default:throw TEXT("异常功能！");
		}
	}
	catch (LPCTSTR lpError) {
		this->m_szInfo[LAST_ERROR] = lpError;
		return false;
	}
	this->m_szInfo[LAST_ERROR] = TEXT("操作成功执行。");
	return true;
}

void WndEngine::UpDateInfo()
{
	m_dwhWndTid = GetWindowThreadProcessId(m_hCWnd, &m_dwProcPid);

	GetWindowRect(m_hCWnd, &m_CurWinRect[0]);
	//GetClientRect(m_hCWnd, &m_CurWinRect[1]);

	//enum Rect{
	//	RECT_WINDOW,RECT_CLIENT,RECT_RESTORE,
	//	RECT_ALL
	//};
	WINDOWINFO	CWINDOWINFO;
	GetWindowInfo(m_hCWnd, &CWINDOWINFO);

	m_CurWinRect[RECT_CLIENT] = CWINDOWINFO.rcClient;
	//m_CurWinRect[RECT_RESTORE] = CWINDOWINFO.rcWindow;


	//m_CurWinRect[RECT_CLIENT].left = 
	//	(m_CurWinRect[0].right-m_CurWinRect[0].left)/2
	//	-(m_CurWinRect[RECT_CLIENT].right-m_CurWinRect[RECT_CLIENT].left)/2;
	//m_CurWinRect[RECT_CLIENT].top = 
	//	(m_CurWinRect[0].bottom-m_CurWinRect[0].top)
	//	-(m_CurWinRect[RECT_CLIENT].bottom-m_CurWinRect[RECT_CLIENT].top)
	//	-m_CurWinRect[RECT_CLIENT].left;
	//m_CurWinRect[RECT_CLIENT].right += m_CurWinRect[RECT_CLIENT].left;
	//m_CurWinRect[RECT_CLIENT].bottom += m_CurWinRect[RECT_CLIENT].top;

	TCHAR szTemp[MAX_PATH];
	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%#08x|%d"), m_hCWnd, m_hCWnd);
	m_szInfo[WND_HANDLE] = szTemp;

	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%#08x"), ::GetMenu(m_hCWnd));
	m_szInfo[WND_MENU] = szTemp;

	if (m_hCWnd == m_hDWnd)
		m_szInfo[WND_TEXT] = TEXT("[DeskTop]");
	else
	{
		DWORD_PTR	dwRet = 0;
		LRESULT ret = NULL;
		ret = SendMessageTimeout(m_hCWnd, WM_GETTEXT, WPARAM(sizeof(szTemp) / sizeof(TCHAR)), LPARAM(szTemp), SMTO_NORMAL, 100, &dwRet);
		m_szInfo[WND_TEXT] = (0 == ret) ? TEXT("[获取标题超时]") : szTemp;
	}

	GetClassName(m_hCWnd, szTemp, sizeof(szTemp) / sizeof(TCHAR));
	m_szInfo[WND_CLSNAME] = szTemp;

	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%d"), GetClassWord(m_hCWnd, GCW_ATOM));
	m_szInfo[WND_CLSVALUE] = szTemp;

	this->GetWndParentInfo();
	this->GetWndStyleInfo();
	this->GetWindowClass();
	this->GetWndModuleInfo();
}

bool WndEngine::GetWndModuleInfo()
{
	bool bSuccess = false;

	TCHAR szTemp[MAX_PATH];
	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%d|%#08x"), m_dwProcPid, m_dwProcPid);
	m_szInfo[WND_PROCESSID] = szTemp;

	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%d|%#08x"), m_dwhWndTid, m_dwhWndTid);
	m_szInfo[PROC_THREADID] = szTemp;

	static TCHAR szModulePath[MAX_PATH] = TEXT("[NULL]");
	static TCHAR szModuleName[MAX_PATH] = TEXT("[NULL]");
	static TCHAR szProcesPath[MAX_PATH] = TEXT("[NULL]");
	static TCHAR szProcesName[MAX_PATH] = TEXT("[NULL]");

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, m_dwProcPid);

	if (hProcess != NULL)
	{
		TCHAR	szTemp1[MAX_PATH] = TEXT("[NULL]"), szTemp2[MAX_PATH] = TEXT("[NULL]");

		HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(m_hCWnd, GWLP_HINSTANCE);
		StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%#08x"), hInstance);
		m_szInfo[WND_MODULEHINST] = szTemp;

		//if(GetProcessImageFileName(hProcess,szProcesPath, MAX_PATH))
		if (GetModuleFileNameEx(hProcess, NULL, szProcesPath, MAX_PATH))
		{
			_tsplitpath_s(szProcesPath, 0, 0, 0, 0, szTemp1, MAX_PATH, szTemp2, MAX_PATH);
			StringCbPrintf(szProcesName, sizeof(szProcesName), TEXT("%s%s"), szTemp1, szTemp2);
		}
		else
		{
			StringCbPrintf(szProcesPath, sizeof(szProcesPath), TEXT("[%s]"), GetLastErrorInfo());
			StringCbPrintf(szProcesName, sizeof(szProcesName), TEXT("%s"), szProcesPath);
		}
		if (hInstance != NULL)
		{
			if (GetModuleFileNameEx(hProcess, hInstance, szModulePath, MAX_PATH))
			{
				_tsplitpath_s((LPCTSTR)szModulePath, 0, 0, 0, 0, (LPTSTR)szTemp1, MAX_PATH, (LPTSTR)szTemp2, MAX_PATH);
				StringCbPrintf(szModuleName, sizeof(szModuleName), TEXT("%s%s"), szTemp1, szTemp2);
			}
			else
			{
				StringCbPrintf(szModulePath, sizeof(szModulePath), TEXT("[%s]"), GetLastErrorInfo());
				StringCbPrintf(szModuleName, sizeof(szModuleName), TEXT("%s"), szModulePath);
			}
		}
		else
		{
			StringCbPrintf(szModulePath, sizeof(szTemp), TEXT("%s"), szProcesPath);
			StringCbPrintf(szModuleName, sizeof(szTemp), TEXT("%s"), szProcesName);
		}

		DWORD needed;
		if (EnumProcessModules(hProcess, &hInstance, sizeof(hInstance), &needed)) {
			StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%#08x"), hInstance);
			m_szInfo[WND_PEOCESSHINST] = szTemp;
		}
		else
			m_szInfo[WND_PEOCESSHINST] = TEXT("[NULL]");

		bSuccess = true;
		CloseHandle(hProcess);
	}
	else
	{
		StringCbPrintf(szModulePath, sizeof(szModulePath), TEXT("[%s]"), ::GetLastErrorInfo());
		StringCbPrintf(szModuleName, sizeof(szModuleName), TEXT("%s"), szModulePath);
		StringCbPrintf(szProcesPath, sizeof(szProcesPath), TEXT("%s"), szModulePath);
		StringCbPrintf(szProcesName, sizeof(szProcesName), TEXT("%s"), szProcesPath);
	}

	m_szInfo[WND_PROCESSPATH] = szProcesPath;
	m_szInfo[WND_PROCESSNAME] = szProcesName;
	m_szInfo[WND_MODULEPATH] = szModulePath;
	m_szInfo[WND_MODULENAME] = szModuleName;

	return bSuccess;
}

bool WndEngine::GetProcessModules()
{
	DWORD	needed, ret = FALSE;

	if (m_dwProcModPrePid == m_dwProcPid)return true;
	else m_dwProcModPrePid = m_dwProcPid;

	m_szInfo[PRO_MODULES] = TEXT("");

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, m_dwProcPid);

	if (hProcess != NULL)
	{
		if (EnumProcessModulesEx(hProcess, NULL, 0, &needed, LIST_MODULES_ALL) && needed != 0)
		{
			HINSTANCE*	hModules = new HINSTANCE[needed / sizeof(HINSTANCE)];

			if (EnumProcessModulesEx(hProcess, hModules, needed, &needed, LIST_MODULES_ALL))
			{
				TCHAR	szTemp[MAX_PATH] = TEXT("[NULL]");
				for (DWORD_PTR i = 0; i < needed / sizeof(HINSTANCE); i++)
				{
					if (GetModuleFileNameEx(hProcess, hModules[i], szTemp, MAX_PATH))
					{
						m_szInfo[PRO_MODULES] += szTemp;
						m_szInfo[PRO_MODULES] += TEXT("\r\n");
					}
				}
			}
			delete[] hModules;
			ret = TRUE;
		}

		CloseHandle(hProcess);
	}
	return ret != false;
}

void WndEngine::GetWndParentInfo()
{
	m_hPWnd = ::GetParent(m_hCWnd);
	if (m_hPWnd == NULL)
	{
		m_hPWnd = (HWND)GetWindowLongPtr(m_hCWnd, GWLP_HWNDPARENT);
#ifdef _DEBUG
		if (m_hPWnd != NULL)printf("GetWindowLong(GWL_HWNDPARENT)\n");
#endif	
	}
	if (m_hPWnd == NULL && m_hCWnd != m_hDWnd)
	{
		m_hPWnd = m_hDWnd;
	}

	if (m_hPWnd != NULL)
	{
		TCHAR szTemp[MAX_PATH];
		StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%#08x|%d"), m_hPWnd, m_hPWnd);
		m_szInfo[WND_PHANDLE] = szTemp;

		GetClassName(m_hPWnd, szTemp, sizeof(szTemp));
		m_szInfo[WND_PCLSNAME] = szTemp;

		if (m_hPWnd == m_hDWnd)
			m_szInfo[WND_PTEXT] = TEXT("[DeskTop]");
		else
		{
			DWORD_PTR	dwRet = 0;
			LRESULT ret = NULL;
			ret = SendMessageTimeout(m_hPWnd, WM_GETTEXT, WPARAM(MAX_PATH), LPARAM(szTemp), SMTO_NORMAL, 100, &dwRet);
			m_szInfo[WND_PTEXT] = szTemp;

			if (0 == ret)
			{
				m_szInfo[WND_PTEXT] = TEXT("[获取标题超时]");
			}
			//FORWARD_WM_GETTEXT(m_hPWnd,MAX_PATH,m_szInfo[WND_PTEXT].GetBufferSetLength(MAX_PATH),::SendMessage);

		}
	}
	else
	{
		m_szInfo[WND_PHANDLE] = TEXT("");
		m_szInfo[WND_PCLSNAME] = TEXT("");
		m_szInfo[WND_PTEXT] = TEXT("");
	}
}

void WndEngine::GetWindowClass()
{
	TCHAR szTemp[MAX_PATH];
	GetClassName(m_hCWnd, szTemp, MAX_PATH);
	m_szInfo[CLS_NAME] = szTemp;

	LONG Style = GetClassLong(m_hCWnd, GCL_STYLE);
	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%#08x"), Style);
	m_szInfo[CLS_STYLE] = szTemp;

	if (Style&CS_VREDRAW)m_szInfo[CLS_STYLE] += TEXT("|CS_VREDRAW");
	if (Style&CS_HREDRAW)m_szInfo[CLS_STYLE] += TEXT("|CS_HREDRAW");
	if (Style&CS_DBLCLKS)m_szInfo[CLS_STYLE] += TEXT("|CS_DBLCLKS");
	if (Style&CS_OWNDC)m_szInfo[CLS_STYLE] += TEXT("|CS_OWNDC");
	if (Style&CS_CLASSDC)m_szInfo[CLS_STYLE] += TEXT("|CS_CLASSDC");
	if (Style&CS_PARENTDC)m_szInfo[CLS_STYLE] += TEXT("|CS_PARENTDC");
	if (Style&CS_NOCLOSE)m_szInfo[CLS_STYLE] += TEXT("|CS_NOCLOSE");
	if (Style&CS_SAVEBITS)m_szInfo[CLS_STYLE] += TEXT("|CS_SAVEBITS");
	if (Style&CS_BYTEALIGNCLIENT)m_szInfo[CLS_STYLE] += TEXT("|CS_BYTEALIGNCLIENT");
	if (Style&CS_BYTEALIGNWINDOW)m_szInfo[CLS_STYLE] += TEXT("|CS_BYTEALIGNWINDOW");
	if (Style&CS_GLOBALCLASS)m_szInfo[CLS_STYLE] += TEXT("|CS_GLOBALCLASS");


	LONG Atom = GetClassLong(m_hCWnd, GCW_ATOM);
	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%#08x"), Atom);
	m_szInfo[CLS_ATOM] = szTemp;

	LONG hMenu = GetClassLong(m_hCWnd, GCLP_MENUNAME);
	StringCbPrintf(szTemp, sizeof(szTemp), hMenu ? TEXT("%#08x") : TEXT("[无]"), hMenu);
	m_szInfo[CLS_MEUN] = szTemp;

	LONG hBrush = GetClassLong(m_hCWnd, GCLP_HBRBACKGROUND);
	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%#08x"), hBrush);
	m_szInfo[CLS_HBRUSH] = szTemp;

	LONG hCursor = GetClassLong(m_hCWnd, GCLP_HCURSOR);
	StringCbPrintf(szTemp, sizeof(szTemp), hCursor ? TEXT("%#08x") : TEXT("[无]"), hCursor);
	m_szInfo[CLS_CURSOR] = szTemp;
	if (hCursor)
	{
		static struct	CURSORSTRING {
			HCURSOR	hCursor;
			LPCTSTR	lpCtstr;
		}sCurStr[] = {
			LoadCursor(NULL,IDC_ARROW),TEXT("IDC_ARROW"),
			LoadCursor(NULL,IDC_IBEAM),TEXT("IDC_IBEAM"),
			LoadCursor(NULL,IDC_WAIT),TEXT("IDC_WAIT"),
			LoadCursor(NULL,IDC_CROSS),TEXT("IDC_CROSS"),
			LoadCursor(NULL,IDC_UPARROW),TEXT("IDC_UPARROW"),
			LoadCursor(NULL,IDC_SIZE),TEXT("IDC_SIZE"),
			LoadCursor(NULL,IDC_ICON),TEXT("IDC_ICON"),
			LoadCursor(NULL,IDC_SIZENWSE),TEXT("IDC_SIZENWSE"),
			LoadCursor(NULL,IDC_SIZENESW),TEXT("IDC_SIZENESW"),
			LoadCursor(NULL,IDC_SIZEWE),TEXT("IDC_SIZEWE"),
			LoadCursor(NULL,IDC_SIZENS),TEXT("IDC_SIZENS"),
			LoadCursor(NULL,IDC_SIZEALL),TEXT("IDC_SIZEALL"),
			LoadCursor(NULL,IDC_HAND),TEXT("IDC_HAND"),
			LoadCursor(NULL,IDC_APPSTARTING),TEXT("IDC_APPSTARTING"),
			LoadCursor(NULL,IDC_HELP),TEXT("IDC_HELP"),
			LoadCursor(NULL,IDC_NO),TEXT("IDC_NO"),
		};
		for (int i = 0, l = sizeof(sCurStr) / sizeof(sCurStr[0]); i<l; i++)
		{
			if (hCursor == LONG_PTR(sCurStr[i].hCursor))
			{
				m_szInfo[CLS_CURSOR] = sCurStr[i].lpCtstr;
				break;
			}
		}

	}
	LONG hIcon = GetClassLong(m_hCWnd, GCLP_HICON);
	StringCbPrintf(szTemp, sizeof(szTemp), hIcon ? TEXT("%#08x") : TEXT("[无]"), hIcon);
	m_szInfo[CLS_ICON] = szTemp;

	LONG hModule = GetClassLong(m_hCWnd, GCLP_HMODULE);
	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%#08x"), hModule);
	m_szInfo[CLS_INSTANCE] = szTemp;

	LONG wExtra = GetClassLong(m_hCWnd, GCL_CBWNDEXTRA);
	StringCbPrintf(szTemp, sizeof(szTemp), wExtra ? TEXT("%#08x") : TEXT("[无]"), wExtra);
	m_szInfo[CLS_BYTE] = szTemp;

	LONG lUserData = GetWindowLong(m_hCWnd, GWLP_USERDATA);
	StringCbPrintf(szTemp, sizeof(szTemp), lUserData ? TEXT("%#08x") : TEXT("[无]"), lUserData);
	m_szInfo[CLS_WNDBYTE] = szTemp;

	LONG pProc = GetClassLong(m_hCWnd, GCLP_WNDPROC);
	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%#08x"), pProc);
	m_szInfo[CLS_WINPROCESS] = szTemp;
}

void WndEngine::GetWndStyleInfo()
{
	LONG 	Style = GetWindowLong(m_hCWnd, GWL_STYLE);
	LONG 	ExStyle = GetWindowLong(m_hCWnd, GWL_EXSTYLE);

	TCHAR szTemp[MAX_PATH];
	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%#08x"), Style);
	m_szInfo[WND_STYLE] = szTemp;
	if (Style&WS_POPUP)m_szInfo[WND_STYLE] += TEXT("|WS_POPUP");
	if (Style&WS_CHILD)m_szInfo[WND_STYLE] += TEXT("|WS_CHILD");
	if (Style&WS_MINIMIZE)m_szInfo[WND_STYLE] += TEXT("|WS_MINIMIZE");
	if (Style&WS_DISABLED)m_szInfo[WND_STYLE] += TEXT("|WS_DISABLED");
	if (Style&WS_CLIPSIBLINGS)m_szInfo[WND_STYLE] += TEXT("|WS_CLIPSIBLINGS");
	if (Style&WS_CLIPCHILDREN)m_szInfo[WND_STYLE] += TEXT("|WS_CLIPCHILDREN");
	if (Style&WS_MAXIMIZE)m_szInfo[WND_STYLE] += TEXT("|WS_MAXIMIZE");
	if ((Style&WS_CAPTION) == WS_CAPTION)m_szInfo[WND_STYLE] += TEXT("|WS_CAPTION");
	if (Style&WS_BORDER)m_szInfo[WND_STYLE] += TEXT("|WS_BORDER");
	if (Style&WS_DLGFRAME)m_szInfo[WND_STYLE] += TEXT("|WS_DLGFRAME");
	if (Style&WS_VSCROLL)m_szInfo[WND_STYLE] += TEXT("|WS_VSCROLL");
	if (Style&WS_HSCROLL)m_szInfo[WND_STYLE] += TEXT("|WS_HSCROLL");
	if (Style&WS_SYSMENU)m_szInfo[WND_STYLE] += TEXT("|WS_SYSMENU");
	if (Style&WS_THICKFRAME)m_szInfo[WND_STYLE] += TEXT("|WS_THICKFRAME");
	if (Style&WS_GROUP)m_szInfo[WND_STYLE] += TEXT("|WS_GROUP");
	if (Style&WS_TABSTOP)m_szInfo[WND_STYLE] += TEXT("|WS_TABSTOP");
	if (Style&WS_MINIMIZEBOX)m_szInfo[WND_STYLE] += TEXT("|WS_MINIMIZEBOX");
	if (Style&WS_MAXIMIZEBOX)m_szInfo[WND_STYLE] += TEXT("|WS_MAXIMIZEBOX");
	if (Style&WS_VISIBLE)m_szInfo[WND_STYLE] += TEXT("|WS_VISIBLE");

	StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%#08x"), ExStyle);
	m_szInfo[WND_EXSTYLE] = szTemp;
	if (ExStyle&WS_EX_DLGMODALFRAME)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_DLGMODALFRAME");
	if (ExStyle&WS_EX_NOPARENTNOTIFY)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_NOPARENTNOTIFY");
	if (ExStyle&WS_EX_TOPMOST)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_TOPMOST");
	if (ExStyle&WS_EX_ACCEPTFILES)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_ACCEPTFILES");
	if (ExStyle&WS_EX_TRANSPARENT)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_TRANSPARENT");
	if (ExStyle&WS_EX_MDICHILD)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_MDICHILD");
	if (ExStyle&WS_EX_TOOLWINDOW)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_TOOLWINDOW");
	if (ExStyle&WS_EX_WINDOWEDGE)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_WINDOWEDGE");
	if (ExStyle&WS_EX_CLIENTEDGE)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_CLIENTEDGE");
	if (ExStyle&WS_EX_CONTEXTHELP)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_CONTEXTHELP");
	if (ExStyle&WS_EX_CONTROLPARENT)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_CONTROLPARENT");
	if (ExStyle&WS_EX_STATICEDGE)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_STATICEDGE");
	if (ExStyle&WS_EX_APPWINDOW)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_APPWINDOW");
	if (ExStyle&WS_EX_LAYERED)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_LAYERED");

	if (ExStyle&WS_EX_RTLREADING)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_RTLREADING");
	//else m_szInfo[WND_EXSTYLE] += TEXT("|[WS_EX_LTRREADING]");
	//if (ExStyle&WS_EX_LTRREADING)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_LTRREADING");
	if (ExStyle&WS_EX_LEFTSCROLLBAR)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_LEFTSCROLLBAR");
	//else m_szInfo[WND_EXSTYLE] += TEXT("|[WS_EX_RIGHTSCROLLBAR]");
	//if (ExStyle&WS_EX_RIGHTSCROLLBAR)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_RIGHTSCROLLBAR");
	if (ExStyle&WS_EX_RIGHT)m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_RIGHT");
	//else m_szInfo[WND_EXSTYLE] += TEXT("|[WS_EX_LEFT]");
	//if (ExStyle&WS_EX_LEFT  )m_szInfo[WND_EXSTYLE] += TEXT("|WS_EX_LEFT");

}

bool WndEngine::UpPrivilege()
{
	HANDLE		hToken = NULL;		//   handle   to   process   token 
	TOKEN_PRIVILEGES   tkp;			//   pointer   to   token   structure 
	BOOL result = OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		&hToken);
	if (!result)return result != FALSE;		//打开进程错误 

	result = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);

	if (!result)return result != FALSE;		//查看进程权限错误 

	tkp.PrivilegeCount = 1;			//   one   privilege   to   set 
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	result = AdjustTokenPrivileges(hToken, FALSE, &tkp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);
	return   result != FALSE;
}

bool WndEngine::IsValidWindow()
{
	return ::IsWindow(this->m_hCWnd) != FALSE;
}

bool WndEngine::QueryProcess(DWORD dwPid)
{
	m_hCWnd = NULL;
	m_dwProcPid = dwPid;

#ifndef _DEBUG
	if (dwPid == GetCurrentProcessId())
	{
		m_dwProcPid = NULL;
		m_dwProcModPrePid = NULL;
		for (int i = 0; i < all; i++)
			m_szInfo[i].clear();
		this->m_szInfo[LAST_ERROR] = TEXT("不能对本进程进行操作。");
		return false;
	}
#endif
	if (this->GetWndModuleInfo() != true)
	{
		for (int i = 0; i < all; i++)
			m_szInfo[i].clear();
		this->m_szInfo[LAST_ERROR] = GetLastErrorInfo();
		return false;
	}
	return true;
}

bool WndEngine::QueryWindow(HWND hWnd)
{
	if (::IsWindow(hWnd) == FALSE)
	{
		for (int i = 0; i < all; i++)
			m_szInfo[i].clear();
		this->m_szInfo[LAST_ERROR] = TEXT("窗口不存在。");
		return false;
	}

	DWORD		dwPid;
	GetWindowThreadProcessId(hWnd, &dwPid);
	if (dwPid == GetCurrentProcessId())
	{
		for (int i = 0; i < all; i++)
			m_szInfo[i].clear();
		this->m_szInfo[LAST_ERROR] = TEXT("窗口属于本进程。");
		return false;
	}
	m_dwProcPid = dwPid;

	if (m_hCWnd != hWnd)
	{
		m_hCWnd = hWnd;
		GetWindowRect(m_hCWnd, &m_CurWinRect[RECT_RESTORE]);
	}

	m_hCWnd = hWnd;
	this->UpDateInfo();
	this->m_szInfo[LAST_ERROR] = TEXT("操作成功执行。");
	return true;
}

void WndEngine::EnumWindowChlid(HWND hWnd, int state)
{
	if (::IsWindow(hWnd) != FALSE)
	{
		HWND	hCWnd = hWnd;
		TCHAR	szText[MAX_PATH], szClass[MAX_PATH], szTitle[MAX_PATH];
		DWORD	dwWndPid, dwThisPid = GetCurrentProcessId();
		do {
			GetWindowThreadProcessId(hCWnd, &dwWndPid);

			if (dwThisPid == dwWndPid)
			{
				hCWnd = ::GetWindow(hCWnd, GW_HWNDNEXT);
				continue;
			}

			::GetWindowText(hCWnd, szText, MAX_PATH);
			::GetClassName(hCWnd, szClass, MAX_PATH);

			StringCchPrintf(szTitle, MAX_PATH, TEXT("%#08x|%s|%s\n"), hCWnd, szText, szClass);

			m_szWindows += szTitle;

#ifdef _DEBUG
			for (int i = 0; i < state; i++)
				_tprintf(TEXT("    "));
			_tprintf(TEXT("%s"), szTitle);
#endif
			EnumWindowChlid(::GetWindow(hCWnd, GW_CHILD), state + 1);

			hCWnd = ::GetWindow(hCWnd, GW_HWNDNEXT);
		} while (hCWnd != NULL);
	}
}

HWND WndEngine::FindWindowEx(HWND hPWnd, HWND hCWndAfter, LPCTSTR lpClass, LPCTSTR lpWnd, bool bCaseMatters, bool bIsFuzzy)
{
	if (lpClass == NULL && lpWnd == NULL)
	{
		this->m_szInfo[LAST_ERROR] = TEXT("关键字不能为空。");
		return NULL;
	}
	
	if (m_szWindows.empty() == true || hCWndAfter == NULL)
	{
		m_szWindows = TEXT("");
		this->EnumWindowChlid(::GetDesktopWindow());
	}

	HWND	hFWnd = NULL;
	bool	bFinding = false;
	size_t	index1 = 0, index2 = 0;
	tstring	szWndInfo, szText, szClass;
	tstring	szArgText = lpWnd, szArgClass = lpClass;

	if (hCWndAfter == NULL)
	{
		bFinding = true;
	}
	if (bCaseMatters == false)
	{
		CharLowerBuff((LPTSTR)szArgText.c_str(), DWORD(szArgText.length()));
		CharLowerBuff((LPTSTR)szArgClass.c_str(), DWORD(szArgClass.length()));
	}

	for (size_t iCom, i = 0, n = 0; (i = m_szWindows.find(TEXT('\n'), n)) != -1; n = i + 1)
	{
		if (i > n)
		{
			szWndInfo = m_szWindows.substr(n, i - n);
#ifdef _WIN64
			hFWnd = HWND(_tcstoll(szWndInfo.c_str(), NULL, 16));
#else
			hFWnd = HWND(_tcstol(szWndInfo.c_str(), NULL, 16));
#endif // _WIN64

			if (hCWndAfter != NULL && hFWnd == hCWndAfter)
			{
				bFinding = true;
				continue;
			}

			if (bFinding == true)
			{
				if (hPWnd == NULL || ::GetParent(hFWnd) == hPWnd)
				{
					index1 = szWndInfo.find('|');
					index2 = szWndInfo.find('|', index1 + 1);

					if (lpWnd != NULL)
					{
						szText = szWndInfo.substr(index1 + 1, index2 - index1 - 1);
						if (bCaseMatters == false)
						{
							CharLowerBuff((LPTSTR)szText.c_str(), DWORD(szText.length()));
						}
						if (bIsFuzzy == true)
							iCom = ::_tcsstr(szText.c_str(), szArgText.c_str()) == NULL;
						else
							iCom = ::lstrcmp(szText.c_str(), lpWnd);
						if (iCom != 0)continue;
					}
					if (lpClass != NULL)
					{
						
						//szClass = szWndInfo.Right(szWndInfo.length() - index2 - 1);
						szClass = szWndInfo.substr(szWndInfo.length() - index2 - 1, index2 + 1).c_str();
						if (bCaseMatters == false)
						{
							CharLowerBuff((LPTSTR)szClass.c_str(), DWORD(szClass.length()));
						}
						if (bIsFuzzy == true)
							iCom = ::_tcsstr(szClass.c_str(), szArgClass.c_str()) == NULL;
						else
							iCom = ::lstrcmp(szClass.c_str(), lpClass);
						if (iCom == 0)return hFWnd;
					}
					else if (lpWnd != NULL && iCom == 0)
					{
						return hFWnd;
					}
				}
			}
		}
	}

	return NULL;
}

bool WndEngine::SetPriorityClass(DWORD dwPid, DWORD dwPriorityClass)
{
	if (dwPid == GetCurrentProcessId())
	{
		m_dwProcPid = NULL;
		m_dwProcModPrePid = NULL;
		for (int i = 0; i < all; i++)
			m_szInfo[i].clear();
		this->m_szInfo[LAST_ERROR] = TEXT("不能对本进程进行操作。");
		return false;
	}

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, dwPid);

	if (hProcess == NULL) {
		this->m_szInfo[LAST_ERROR] = GetLastErrorInfo();
		return false;
	}
	if (FALSE == ::SetPriorityClass(hProcess, dwPriorityClass))
	{
		this->m_szInfo[LAST_ERROR] = GetLastErrorInfo();
		return false;
	}

	CloseHandle(hProcess);
	return true;
}

bool WndEngine::GetPriorityClass(DWORD dwPid, LPDWORD lpPriorityClass)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, dwPid);

	if (hProcess == NULL) {
		this->m_szInfo[LAST_ERROR] = GetLastErrorInfo();
		return false;
	}

	lpPriorityClass[0] = ::GetPriorityClass(hProcess);

	if (lpPriorityClass[0] == 0) {
		this->m_szInfo[LAST_ERROR] = GetLastErrorInfo();
		CloseHandle(hProcess);
		return false;
	}

	CloseHandle(hProcess);
	return true;
}
