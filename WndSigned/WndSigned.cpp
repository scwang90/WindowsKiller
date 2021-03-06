// WndSigned.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include "WndFrame.h"
#include "WndSigned.h"
#include <windowsx.h>

// class IWndSigned implementation

ULONG    g_LockNumber = 0;
ULONG    g_IWndSignedNumber = 0;
HANDLE	 g_hModule;

extern BOOL g_bIsSetRect;

WndSigned::WndSigned(void)
{
	//////////////////////////////////////////////////////////////////////////////
	//	Unknown
	m_Ref = 0;
	g_IWndSignedNumber++;
	//////////////////////////////////////////////////////////////////////////////
	//	IWndSigned

	HINSTANCE hInstance = GetModuleHandle(TEXT("WndSigned.dll"));

	// Register Class 
	MyRegisterClass(hInstance);

	// Perform application initialization:
	m_hWnd = InitInstance(hInstance, SW_HIDE);

	if (m_hWnd == NULL)
	{
		MessageBox(NULL, TEXT("创建标记窗口失败"), ::GetLastErrorInfo(TEXT("创建标记窗口失败")), MB_ICONERROR | MB_TOPMOST);
	}
}

WndSigned::~WndSigned(void)
{
	DestroyWindow(m_hWnd);
}

HRESULT DllWndSigned::DllCoCreateObject(const CLSID& clsid, const IID& iid, void **ppv)
{
	if (clsid == CLSID_IWndSigned) {

		WndSigned *pFactory = new WndSigned;

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


HRESULT	WndSigned::QueryInterface(const IID& iid, void **ppv)
{
	if (iid == IID_IUnknown)
	{
		*ppv = (IUnknown *)this;
		((IUnknown *)(*ppv))->AddRef();
	}
	else if (iid == IID_IWndSigned)
	{
		*ppv = (IWndSigned *)this;
		((IWndSigned *)(*ppv))->AddRef();
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG	WndSigned::AddRef()
{
	m_Ref++;
	return  (ULONG)m_Ref;
}

ULONG	WndSigned::Release()
{
	m_Ref--;
	if (m_Ref == 0) {
		g_IWndSignedNumber--;
		delete this;
		return 0;
	}
	return  (ULONG)m_Ref;
}
bool WndSigned::SignHide()
{
	KillTimer(m_hWnd, IDT_SHOW);
	ShowWindow(m_hWnd, SW_HIDE);

	return true;
}
bool WndSigned::SignRect(LPRECT lpRect, int iTime)
{
	if (lpRect->right < lpRect->left
		|| lpRect->bottom < lpRect->top)return false;

	g_bIsSetRect = TRUE;

	::MoveWindow(m_hWnd,
		lpRect->left, lpRect->top,
		lpRect->right - lpRect->left,
		lpRect->bottom - lpRect->top,
		TRUE);

	if (IsWindowVisible(m_hWnd) == FALSE)
	{
		::SetWindowPos(m_hWnd, 0, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		::SetTimer(m_hWnd, IDT_SHOW, 400, NULL);
	}
	else
	{
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	}

	if (iTime != 0)::SetTimer(m_hWnd, IDT_TIMEOUT, iTime, NULL);

	return true;
}
bool WndSigned::IsSignHide()
{
	return IsWindowVisible(m_hWnd) == FALSE;
}
bool WndSigned::SetSignColor(DWORD color)
{
	extern	HPEN	g_hSignPen;
	extern	DWORD	g_SignColor;

	DeleteObject(g_hSignPen);
	g_hSignPen = CreatePen(PS_SOLID, 4, g_SignColor = color);

	return	true;
}
LONG WndSigned::GetSignColor()
{
	extern	DWORD	g_SignColor;
	return	g_SignColor;
}
