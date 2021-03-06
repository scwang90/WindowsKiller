// WndAttGeneralDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndAttGeneralDlg.h"
#include "afxdialogex.h"


// CWndAttGeneralDlg 对话框

IMPLEMENT_DYNAMIC(CWndAttGeneralDlg, CDialog)

CWndAttGeneralDlg::CWndAttGeneralDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_GENERAL, pParent)
{

}

CWndAttGeneralDlg::~CWndAttGeneralDlg()
{
}

BOOL CWndAttGeneralDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	ASSERT(IS_INTRESOURCE(lpszTemplateName) || AfxIsValidString(lpszTemplateName));

	m_lpszTemplateName = lpszTemplateName;  // used for help
	if (IS_INTRESOURCE(m_lpszTemplateName) && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD_PTR)m_lpszTemplateName);

	HINSTANCE hInst = ::GetModuleHandle(NULL/*TEXT("WndAttribute.dll")*/);
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);

	BOOL bResult = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);
	UnlockResource(hTemplate);
	FreeResource(hTemplate);

	return bResult;
}
void CWndAttGeneralDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWndAttGeneralDlg, CDialog)

	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CWndAttGeneralDlg::OnBnClickedButtonOkCancel)
	ON_BN_CLICKED(IDOK, &CWndAttGeneralDlg::OnBnClickedButtonOkCancel)

END_MESSAGE_MAP()


// CWndAttGeneralDlg 消息处理程序

BOOL CWndAttGeneralDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CWndAttGeneralDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}
BOOL CWndAttGeneralDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int nID = LOWORD(wParam);
	HWND hCtl = HWND(lParam);
	UINT code = UINT(HIWORD(wParam));
	if (code == EN_SETFOCUS)
	{
		TCHAR	szClassName[128] = TEXT("");
		GetClassName(hCtl, szClassName, 128);
		if (lstrcmpi(szClassName, TEXT("edit")) == 0)
		{
			CEdit	cEdit;
			cEdit.Attach(hCtl);
			cEdit.SetSel(0, -1);
			cEdit.Detach();
		}
	}

	return CWnd::OnCommand(wParam, lParam);
}

void CWndAttGeneralDlg::OnClose()
{
	CDialog::OnClose();
	// TODO: 在此添加消息处理程序代码和/或调用默认值

}

void CWndAttGeneralDlg::OnBnClickedButtonOkCancel()
{
}
bool CWndAttGeneralDlg::SetIWndEngine(IWndEngine* pIWndEngine)
{
	if (pIWndEngine != NULL)
	{
		mPtrWndEngine = pIWndEngine;
		return true;
	}
	return false;
}
void CWndAttGeneralDlg::UpdateWindowInfo()
{
#ifdef _DEBUG
	ASSERT(mPtrWndEngine != NULL);
#endif
	static	DWORD	ArrayItemId[] = {
		IDC_EDIT_WINCAPTION,IDC_EDIT_WINHANDLE,IDC_EDIT_INSTHANDLE,
		IDC_EDIT_WINPROC,IDC_EDIT_MENUHANDLE
	};
	static	DWORD	ArrayInterface[] = {
		IWndEngine::WND_TEXT,IWndEngine::WND_HANDLE,IWndEngine::WND_MODULEHINST,
		IWndEngine::CLS_WINPROCESS,IWndEngine::WND_MENU
	};


	for (int i = 0, len = sizeof(ArrayItemId) / sizeof(ArrayItemId[0]); i<len; i++)
	{
		SetDlgItemText(ArrayItemId[i], mPtrWndEngine->GetStageInfo(ArrayInterface[i]));
	}
	UpdateWindowRect(mPtrWndEngine->GetWndRect(IWndEngine::RECT_WINDOW), IDC_EDIT_WINRECTANGLE);
	UpdateWindowRect(mPtrWndEngine->GetWndRect(IWndEngine::RECT_RESTORE), IDC_EDIT_WINRESTORERECT);
	UpdateWindowRect(mPtrWndEngine->GetWndRect(IWndEngine::RECT_CLIENT), IDC_EDIT_WINCLIENTRECT);
}
void CWndAttGeneralDlg::UpdateWindowRect(LPRECT lpRect, DWORD iTemId)
{
	CString	szTemp;
	szTemp.Format(TEXT("(%d,%d)-(%d,%d),%dx%d"),
		lpRect->left, lpRect->top, lpRect->right, lpRect->bottom,
		lpRect->right - lpRect->left, lpRect->bottom - lpRect->top);
	SetDlgItemText(iTemId, szTemp);
}
