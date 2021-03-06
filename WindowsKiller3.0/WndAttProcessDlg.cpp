// WndAttProcessDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndAttProcessDlg.h"
#include "afxdialogex.h"


// CWndAttProcessDlg 对话框

IMPLEMENT_DYNAMIC(CWndAttProcessDlg, CDialog)

CWndAttProcessDlg::CWndAttProcessDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_PROCESS, pParent)
{

}

CWndAttProcessDlg::~CWndAttProcessDlg()
{
}

BOOL CWndAttProcessDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
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
void CWndAttProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PROCESSID, m_ProcessId);
	DDX_Control(pDX, IDC_STATIC_THREADID, m_ThreadId);
}

BEGIN_MESSAGE_MAP(CWndAttProcessDlg, CDialog)

	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CWndAttProcessDlg::OnBnClickedButtonOkCancel)
	ON_BN_CLICKED(IDOK, &CWndAttProcessDlg::OnBnClickedButtonOkCancel)

END_MESSAGE_MAP()


// CWndAttProcessDlg 消息处理程序

BOOL CWndAttProcessDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CWndAttProcessDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}
BOOL CWndAttProcessDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{

	return CWnd::OnCommand(wParam, lParam);
}

void CWndAttProcessDlg::OnClose()
{
	CDialog::OnClose();
	// TODO: 在此添加消息处理程序代码和/或调用默认值

}

void CWndAttProcessDlg::OnBnClickedButtonOkCancel()
{
}
bool CWndAttProcessDlg::SetIWndEngine(IWndEngine* pIWndEngine)
{
	if (pIWndEngine != NULL)
	{
		mPtrWndEngine = pIWndEngine;
		return true;
	}
	return false;
}
void CWndAttProcessDlg::UpdateWindowInfo()
{
#ifdef _DEBUG
	ASSERT(mPtrWndEngine != NULL);
#endif

	SetDlgItemText(IDC_STATIC_PROCESSID, mPtrWndEngine->GetStageInfo(IWndEngine::PROC_PROCESSID));
	SetDlgItemText(IDC_STATIC_THREADID, mPtrWndEngine->GetStageInfo(IWndEngine::PROC_THREADID));
}
