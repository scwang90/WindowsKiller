// WndAttStylesDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndAttStylesDlg.h"
#include "afxdialogex.h"


// CWndAttStylesDlg 对话框

IMPLEMENT_DYNAMIC(CWndAttStylesDlg, CDialog)

CWndAttStylesDlg::CWndAttStylesDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_STYLES, pParent)
{

}

CWndAttStylesDlg::~CWndAttStylesDlg()
{
}

BOOL CWndAttStylesDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	ASSERT(IS_INTRESOURCE(lpszTemplateName) || AfxIsValidString(lpszTemplateName));

	m_lpszTemplateName = lpszTemplateName;  // used for help
	if (IS_INTRESOURCE(m_lpszTemplateName) && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD_PTR)m_lpszTemplateName);

	HINSTANCE hInst = ::GetModuleHandle(TEXT("WndAttribute.dll"));
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);

	BOOL bResult = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);
	UnlockResource(hTemplate);
	FreeResource(hTemplate);

	return bResult;
}
void CWndAttStylesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_STYLE, m_ListStyle);
	DDX_Control(pDX, IDC_LIST_EXSTYLE, m_ListExStyle);
}

BEGIN_MESSAGE_MAP(CWndAttStylesDlg, CDialog)

	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CWndAttStylesDlg::OnBnClickedButtonOkCancel)
	ON_BN_CLICKED(IDOK, &CWndAttStylesDlg::OnBnClickedButtonOkCancel)

END_MESSAGE_MAP()


// CWndAttStylesDlg 消息处理程序

BOOL CWndAttStylesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CWndAttStylesDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}
BOOL CWndAttStylesDlg::OnCommand(WPARAM wParam, LPARAM lParam)
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

void CWndAttStylesDlg::OnClose()
{
	CDialog::OnClose();
	// TODO: 在此添加消息处理程序代码和/或调用默认值

}

void CWndAttStylesDlg::OnBnClickedButtonOkCancel()
{
}
bool CWndAttStylesDlg::SetIWndEngine(IWndEngine* pIWndEngine)
{
	if (pIWndEngine != NULL)
	{
		mPtrWndEngine = pIWndEngine;
		return true;
	}
	return false;
}
void CWndAttStylesDlg::UpdateWindowInfo()
{
#ifdef _DEBUG
	ASSERT(mPtrWndEngine != NULL);
#endif
	m_ListStyle.ResetContent();
	m_ListExStyle.ResetContent();

	CString szStyle = mPtrWndEngine->GetStageInfo(IWndEngine::WND_STYLE);
	CString szExStyle = mPtrWndEngine->GetStageInfo(IWndEngine::WND_EXSTYLE);

	szStyle += '|';
	szExStyle += '|';

	UpdateUI(IDC_EDIT_WINSTYLE, m_ListStyle, szStyle);
	UpdateUI(IDC_EDIT_WINEXSTYLE, m_ListExStyle, szExStyle);
}
void CWndAttStylesDlg::UpdateUI(DWORD ItemId, CListBox &cListBox, CString &cString)
{
	int n = cString.Find('|');
	SetDlgItemText(ItemId, cString.Left(n));
	for (int i = 0; (i = cString.Find('|', n)) != -1; n = i + 1)
	{
		if (i > n)
			cListBox.AddString(cString.Mid(n, i - n));
	}
}
