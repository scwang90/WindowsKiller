#include "stdafx.h"
#include "WndExtractMgr.h"


// class IWndExtract implementation

CWndExtractMgr::CWndExtractMgr()
{
	//////////////////////////////////////////////////////////////////////////////
	//	Unknown
	m_Ref = 0;
	//////////////////////////////////////////////////////////////////////////////
	//	IWndExtract
	m_BorderNum = 0;
	memset(m_lpBorder, 0, sizeof(m_lpBorder));
}


CWndExtractMgr::~CWndExtractMgr()
{
	for (int i = 0; i < m_BorderNum; i++)
		delete	m_lpBorder[i];
}

bool CWndExtractMgr::AddBorderWnd(HWND hwnd)
{
	if (::IsWindow(hwnd) == FALSE)return false;

	if (m_BorderNum >= MAX_BORDER)
	{
		AfxMessageBox(TEXT("超过窗口最大数目"), MB_ICONERROR | MB_TOPMOST);
		return false;
	}
	for (int i = 0; i < m_BorderNum; i++)
	{
		if (m_lpBorder[i]->GetChild() == hwnd)
		{
			return false;
		}
	}
	m_lpBorder[m_BorderNum++] = new CWndExtBorderDlg(this, hwnd, NULL);
	return true;
}
bool CWndExtractMgr::DelBorderWnd(void* lpVoid)
{
	for (int i = 0; i < m_BorderNum; i++)
	{
		if ((void*)m_lpBorder[i] == lpVoid)
		{
			delete	m_lpBorder[i];
			m_lpBorder[i] = m_lpBorder[--m_BorderNum];
			return true;
		}
	}
	return false;
}

HRESULT  CWndExtractMgr::QueryInterface(const IID& iid, void **ppv)
{
	if (iid == IID_IUnknown)
	{
		*ppv = (IUnknown *)this;
		((IUnknown *)(*ppv))->AddRef();
	}
	else if (iid == IID_IWndExtract)
	{
		*ppv = (IWndExtract *)this;
		((IWndExtract *)(*ppv))->AddRef();
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG	  CWndExtractMgr::AddRef()
{
	m_Ref++;
	return  (ULONG)m_Ref;
}

ULONG	  CWndExtractMgr::Release()
{
	m_Ref--;
	if (m_Ref == 0) {
		delete this;
		return 0;
	}
	return  (ULONG)m_Ref;
}
