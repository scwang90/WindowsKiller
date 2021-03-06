// WndAttWindowsDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndAttWindowDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	WM_CHECKWINDOW	(WM_USER+256)

IMPLEMENT_DYNAMIC(CWndAttWindowDlg, CDialog)

CWndAttWindowDlg::~CWndAttWindowDlg()
{

}

CWndAttWindowDlg::CWndAttWindowDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_WINDOWS, pParent)
{

}

BOOL CWndAttWindowDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	ASSERT(IS_INTRESOURCE(lpszTemplateName) || AfxIsValidString(lpszTemplateName));

	m_lpszTemplateName = lpszTemplateName;  // used for help
	if (IS_INTRESOURCE(m_lpszTemplateName) && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD_PTR)m_lpszTemplateName);

	HINSTANCE hInst = ::GetModuleHandle(NULL/*,TEXT("WndAttribute.dll")*/);
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);

	BOOL bResult = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);
	UnlockResource(hTemplate);
	FreeResource(hTemplate);

	return bResult;
}
void CWndAttWindowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_HANDLE_NEXT, m_HangleNext);
	DDX_Control(pDX, IDC_STATIC_HANDLE_PREV, m_HanglePrev);
	DDX_Control(pDX, IDC_STATIC_HANDLE_PARENT, m_HangleParent);
	DDX_Control(pDX, IDC_STATIC_HANDLE_FIRSTCHILD, m_HangleFirst);
	DDX_Control(pDX, IDC_STATIC_HANDLE_OWNER, m_HangleOwner);
}

BEGIN_MESSAGE_MAP(CWndAttWindowDlg, CDialog)

	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CWndAttWindowDlg::OnBnClickedButtonOkCancel)
	ON_BN_CLICKED(IDOK, &CWndAttWindowDlg::OnBnClickedButtonOkCancel)

END_MESSAGE_MAP()


// CWndAttWindowDlg 消息处理程序

BOOL CWndAttWindowDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO: 在此添加额外的初始化代码

	m_HangleNext.SetToolTipText(TEXT("转到下一个窗口"));
	m_HangleNext.SetTextColor(RGB(0, 0, 255), RGB(255, 0, 0));
	m_HanglePrev.SetToolTipText(TEXT("转到上一个窗口"));
	m_HanglePrev.SetTextColor(RGB(0, 0, 255), RGB(255, 0, 0));
	m_HangleParent.SetToolTipText(TEXT("转到父窗口"));
	m_HangleParent.SetTextColor(RGB(0, 0, 255), RGB(255, 0, 0));
	m_HangleFirst.SetToolTipText(TEXT("转到子窗口"));
	m_HangleFirst.SetTextColor(RGB(0, 0, 255), RGB(255, 0, 0));
	m_HangleOwner.SetToolTipText(TEXT("转到所有者窗口"));
	m_HangleOwner.SetTextColor(RGB(0, 0, 255), RGB(255, 0, 0));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CWndAttWindowDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}
BOOL CWndAttWindowDlg::OnCommand(WPARAM wParam, LPARAM lParam)
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
	else if (code == BN_CLICKED)
	{
		static DWORD dwHandleIds[] = {
			IDC_STATIC_HANDLE_NEXT,
			IDC_STATIC_HANDLE_PREV,
			IDC_STATIC_HANDLE_PARENT,
			IDC_STATIC_HANDLE_FIRSTCHILD,
			IDC_STATIC_HANDLE_OWNER
		};
		static DWORD dwFunIds[] = {
			IWndEngine::GETWINDOW_NEXT,
			IWndEngine::GETWINDOW_PREV,
			IWndEngine::GETWINDOW_PARENT,
			IWndEngine::GETWINDOW_FIRSTCHILD,
			IWndEngine::GETWINDOW_OWNER
		};

		for (int i = 0; i < sizeof(dwFunIds) / sizeof(dwFunIds[0]); i++)
			if (nID == dwHandleIds[i])
			{
				mPtrWndEngine->QueryWindow(mPtrWndEngine->GetWindow(dwFunIds[i]));
				::SendMessage(GetParent()->GetParent()->m_hWnd, WM_CHECKWINDOW, 0, LPARAM(mPtrWndEngine->GetWindow()));
				break;
			}


	}
	return CWnd::OnCommand(wParam, lParam);
}

void CWndAttWindowDlg::OnClose()
{
	CDialog::OnClose();
	// TODO: 在此添加消息处理程序代码和/或调用默认值

}

void CWndAttWindowDlg::OnBnClickedButtonOkCancel()
{
}
bool CWndAttWindowDlg::SetIWndEngine(IWndEngine* pIWndEngine)
{
	if (pIWndEngine != NULL)
	{
		mPtrWndEngine = pIWndEngine;
		return true;
	}
	return false;
}
void CWndAttWindowDlg::UpdateWindowInfo()
{
#ifdef _DEBUG
	ASSERT(mPtrWndEngine != NULL);
#endif

	static DWORD dwHandleIds[] = {
		IDC_STATIC_HANDLE_NEXT,
		IDC_STATIC_HANDLE_PREV,
		IDC_STATIC_HANDLE_PARENT,
		IDC_STATIC_HANDLE_FIRSTCHILD,
		IDC_STATIC_HANDLE_OWNER
	};
	static DWORD dwTitleIds[] = {
		IDC_EDIT_CAPTION_NEXT,
		IDC_EDIT_CAPTION_PREV,
		IDC_EDIT_CAPTION_PARENT,
		IDC_EDIT_CAPTION_FIRSTCHILD,
		IDC_EDIT_CAPTION_OWNER
	};
	static DWORD dwFunIds[] = {
		IWndEngine::GETWINDOW_NEXT,
		IWndEngine::GETWINDOW_PREV,
		IWndEngine::GETWINDOW_PARENT,
		IWndEngine::GETWINDOW_FIRSTCHILD,
		IWndEngine::GETWINDOW_OWNER
	};

	static CLinkStatic* pCLinkStatic[] = {
		&m_HangleNext,&m_HanglePrev,
		&m_HangleParent,&m_HangleFirst,&m_HangleOwner,
	};

	HWND hWnd = NULL;
	TCHAR szBuffer[MAX_PATH];

	for (int i = 0, len = sizeof(dwFunIds) / sizeof(dwFunIds[0]); i < len; i++)
	{
		hWnd = mPtrWndEngine->GetWindow(dwFunIds[i]);

		if (hWnd != NULL)
		{
			pCLinkStatic[i]->m_bLinkStatic = TRUE;
			StringCchPrintf(szBuffer, MAX_PATH, TEXT("%#x"), hWnd);
			SetDlgItemText(dwHandleIds[i], szBuffer);

			FORWARD_WM_GETTEXT(hWnd, MAX_PATH, szBuffer, ::SendMessage);
			if (szBuffer[0])
				SetDlgItemText(dwTitleIds[i], szBuffer);
			else
				SetDlgItemText(dwTitleIds[i], TEXT("\"\""));
		}
		else
		{
			pCLinkStatic[i]->m_bLinkStatic = FALSE;
			SetDlgItemText(dwHandleIds[i], TEXT("(无)"));
			SetDlgItemText(dwTitleIds[i], TEXT("\"\""));
		}
	}
}
