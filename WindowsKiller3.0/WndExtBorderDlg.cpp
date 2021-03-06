// WndExtBorderDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndExtBorderDlg.h"
#include "afxdialogex.h"

#define	IDM_SYS_UPDATE		5120
#define	IDM_SYS_TOPMOST		5121
#define	IDM_SYS_HIDEBAR		5122
#define	IDM_SYS_UNTOPMOST	5123
#define	IDM_SYS_SHOWBAR		5124
#define	IDM_SYS_LOCK		5125
#define	IDM_SYS_UNLOCK		5126
#define IDM_SYS_LOCKRECT	5127
#define IDM_SYS_SORPTION	5128

#define	DT_LOCK	1000

// CWndExtBorderDlg 对话框

IMPLEMENT_DYNAMIC(CWndExtBorderDlg, CDialog)

CWndExtBorderDlg::CWndExtBorderDlg(IWndExtract*	pIWndExtract, HWND hWnd, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_BORDER_DIALOG, pParent)
{
	m_bIcon = NULL;
	m_sIcon = NULL;
	m_hCwnd = hWnd;
	m_hPwnd = ::GetParent(hWnd);
	m_pIWndExtract = pIWndExtract;
	m_pITaskbar = NULL;
	m_fTopMost = false;
	m_flock = false;

	m_uElapse = 100;

	if (FAILED(CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_SERVER, IID_ITaskbarList, (LPVOID *)&m_pITaskbar)))
	{
		AfxMessageBox(TEXT("Create CLSID_TaskbarList failed!\n"), MB_ICONERROR | MB_TOPMOST);
		ExitProcess(0);
	}

	::GetWindowRect(m_hCwnd, &m_CRect);

	if (m_hPwnd == NULL)
		m_hPwnd = ::GetDesktopWindow();

	for (HWND hPwnd = m_hCwnd; !m_sIcon && !m_bIcon; )
	{
		m_bIcon = HICON(::SendMessage(hPwnd, WM_GETICON, ICON_BIG, 0));
		m_sIcon = HICON(::SendMessage(hPwnd, WM_GETICON, ICON_SMALL, 0));

		if (!(hPwnd = (HWND)::GetParent(hPwnd))
			&& !(hPwnd = (HWND)::GetWindowLongPtr(hPwnd, GWLP_HWNDPARENT)))
			break;
	}
	for (HWND hPwnd = m_hCwnd; !m_fTopMost; )
	{
		m_fTopMost = (GetWindowExStyle(hPwnd) & WS_EX_TOPMOST) != 0;

		if (!(hPwnd = (HWND)::GetParent(hPwnd))
			&& !(hPwnd = (HWND)::GetWindowLongPtr(hPwnd, GWLP_HWNDPARENT)))
			break;
	}

	CWndExtBorderDlg::Create(ATL_MAKEINTRESOURCE(CWndExtBorderDlg::IDD), pParent);
}

CWndExtBorderDlg::~CWndExtBorderDlg()
{
	this->Restore();
	m_pITaskbar->Release();
}

BOOL CWndExtBorderDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	ASSERT(IS_INTRESOURCE(lpszTemplateName) || AfxIsValidString(lpszTemplateName));

	m_lpszTemplateName = lpszTemplateName;  // used for help
	if (IS_INTRESOURCE(m_lpszTemplateName) && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD_PTR)m_lpszTemplateName);

	HINSTANCE hInst = ::GetModuleHandle(NULL/*TEXT("WndExtract.dll")*/);
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);
	/*
	CRect	cRect = m_CRect;
	AdjustWindowRect(&cRect,WS_CAPTION|WS_BORDER,FALSE);

	DLGTEMPLATE* pDlgTemplate = (DLGTEMPLATE*)lpDialogTemplate;
	printf("pDlgTemplate = {%d,%d,%d,%d}\n",lpDialogTemplate->x,lpDialogTemplate->y,lpDialogTemplate->cy,lpDialogTemplate->cy);
	pDlgTemplate->x		=	cRect.left;
	pDlgTemplate->y		=	cRect.top;
	pDlgTemplate->cx	=	cRect.Width();
	pDlgTemplate->cy	=	cRect.Height();
	printf("pDlgTemplate = {%d,%d,%d,%d}\n",lpDialogTemplate->x,lpDialogTemplate->y,lpDialogTemplate->cy,lpDialogTemplate->cy);
	*/
	BOOL bResult = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);
	UnlockResource(hTemplate);
	FreeResource(hTemplate);

	return bResult;
}
void CWndExtBorderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWndExtBorderDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDCANCEL, &CWndExtBorderDlg::OnBnClickedButtonOkCancel)
	ON_BN_CLICKED(IDOK, &CWndExtBorderDlg::OnBnClickedButtonOkCancel)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CWndExtBorderDlg 消息处理程序

BOOL CWndExtBorderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将菜单项添加到系统菜单中。

	CMenu *pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_SYS_HIDEBAR, CString("隐藏标签"));
		pSysMenu->AppendMenu(MF_STRING, IDM_SYS_TOPMOST, CString("总在前面"));
		pSysMenu->AppendMenu(MF_STRING, IDM_SYS_UPDATE, CString("更新信息"));
		pSysMenu->AppendMenu(MF_STRING, IDM_SYS_LOCK, CString("位置锁定"));
		pSysMenu->AppendMenu(MF_STRING, IDM_SYS_LOCKRECT, CString("设置位置"));
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作

	SetIcon(m_bIcon, TRUE);			// 设置大图标
	SetIcon(m_sIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码


	m_LockWizardDlg.Create(ATL_MAKEINTRESOURCE(CWndExtLockWizardDlg::IDD), this);
	m_LockWizardDlg.SetRLockWizardDlg(this);

	CRect	cRect = m_CRect;
	AdjustWindowRect(&cRect, WS_CAPTION | WS_BORDER, FALSE);

	TCHAR	szTitle[256];
	FORWARD_WM_GETTEXT(m_hCwnd, 256, szTitle, ::SendMessage);

	m_CRect.bottom -= m_CRect.top;
	m_CRect.right -= m_CRect.left;
	::ScreenToClient(m_hPwnd, LPPOINT(&m_CRect));
	m_CRect.bottom += m_CRect.top;
	m_CRect.right += m_CRect.left;

	if (::SetParent(m_hCwnd, m_hWnd) == NULL)
	{
		AfxMessageBox(::GetLastErrorInfo(TEXT("提取窗口失败")), MB_ICONERROR | MB_TOPMOST);
		FORWARD_WM_SYSCOMMAND(m_hWnd, SC_CLOSE, 0, 0, ::SendMessage);
	}
	else
	{
		this->MoveWindow(cRect.left, cRect.top, cRect.Width(), cRect.Height());
		this->SetWindowText(szTitle);
		this->ShowWindow(SW_NORMAL);

		::MoveWindow(m_hCwnd, 0, 0, m_CRect.Width(), m_CRect.Height(), TRUE);

		if (m_fTopMost == true)
		{
			FORWARD_WM_SYSCOMMAND(this->m_hWnd, IDM_SYS_TOPMOST, 0, 0, ::PostMessage);
		}
	}

	GetClientRect(&m_lRect);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CWndExtBorderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID > IDM_SYS_TOPMOST - 1 && nID < IDM_SYS_SORPTION + 1)
	{
		CMenu *pSysMenu = GetSystemMenu(FALSE);
		if (nID == IDM_SYS_TOPMOST)
		{
			::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);

			if ((GetWindowExStyle(m_hWnd) & WS_EX_TOPMOST) && pSysMenu != NULL)
				pSysMenu->ModifyMenu(nID, MF_BYCOMMAND, IDM_SYS_UNTOPMOST, TEXT("取消最顶"));
		}
		else if (nID == IDM_SYS_UNTOPMOST)
		{
			::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);

			if (!(GetWindowExStyle(m_hWnd) & WS_EX_TOPMOST) && pSysMenu != NULL)
			{
				pSysMenu->ModifyMenu(nID, MF_BYCOMMAND, IDM_SYS_TOPMOST, TEXT("总在前面"));
			}

		}
		else if (nID == IDM_SYS_HIDEBAR)
		{
			if (SUCCEEDED(m_pITaskbar->DeleteTab(m_hWnd)) && pSysMenu != NULL)
			{
				SetWindowLongPtr(m_hWnd, GWLP_HWNDPARENT, LONG_PTR(AfxGetApp()->m_pMainWnd->m_hWnd));
				pSysMenu->ModifyMenu(nID, MF_BYCOMMAND, IDM_SYS_SHOWBAR, TEXT("显示标签"));
			}

		}
		else if (nID == IDM_SYS_SHOWBAR)
		{
			if (SUCCEEDED(m_pITaskbar->AddTab(m_hWnd)) && pSysMenu != NULL)
			{
				//SetParent(0);
				SetWindowLongPtr(m_hWnd, GWLP_HWNDPARENT, LONG(0));
				pSysMenu->ModifyMenu(nID, MF_BYCOMMAND, IDM_SYS_HIDEBAR, TEXT("隐藏标签"));
			}

		}
		else if (nID == IDM_SYS_LOCK)
		{
			//m_flock	=	true;
			SetTimer(DT_LOCK, m_uElapse, NULL);
			pSysMenu->ModifyMenu(nID, MF_BYCOMMAND, IDM_SYS_UNLOCK, TEXT("解除锁定"));
			//if(CCbtHookWnd::RegisterHookWnd(m_hCwnd,this))
			//{
			//	this->OnCbtHook(0, 0);
			//	pSysMenu->ModifyMenu(nID,MF_BYCOMMAND,IDM_SYS_UNLOCK,"解除锁定");
			//	m_flock	=	true;
			//}
		}
		else if (nID == IDM_SYS_UNLOCK)
		{
			//m_flock	=	false;
			KillTimer(DT_LOCK);
			pSysMenu->ModifyMenu(nID, MF_BYCOMMAND, IDM_SYS_LOCK, TEXT("位置锁定"));
			//if(CCbtHookWnd::IninstallHookWnd(m_hCwnd))
			//{
			//	pSysMenu->ModifyMenu(nID,MF_BYCOMMAND,IDM_SYS_LOCK,"位置锁定");
			//	m_flock	=	false;
			//}
		}
		else if (nID == IDM_SYS_LOCKRECT)
		{
			m_LockWizardDlg.WizardWndRect(m_hCwnd, m_hWnd);
		}
		else if (nID == IDM_SYS_SORPTION)
		{
			m_flock = false;
			::GetClientRect(m_hWnd, &m_lRect);
			::MoveWindow(m_hCwnd, m_lRect.left, m_lRect.top, m_lRect.Width(), m_lRect.Height(), TRUE);
			//pSysMenu->ModifyMenu(nID,MF_BYCOMMAND,IDM_SYS_LOCKRECT,"位置锁定");
			pSysMenu->DeleteMenu(nID, MF_BYCOMMAND);
		}
	}
	else if (nID == IDM_SYS_UPDATE)
	{
		TCHAR	szTitle[256];
		FORWARD_WM_GETTEXT(m_hCwnd, 256, szTitle, ::SendMessage);
		this->SetWindowText(szTitle);
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}
BOOL CWndExtBorderDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return CWnd::OnCommand(wParam, lParam);
}
// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWndExtBorderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_sIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CWndExtBorderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_sIcon);
}


void CWndExtBorderDlg::OnClose()
{
	CDialog::OnClose();
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_pIWndExtract->DelBorderWnd(this);
}
void CWndExtBorderDlg::Restore()
{
	::MoveWindow(m_hCwnd, m_CRect.left, m_CRect.top, m_CRect.Width(), m_CRect.Height(), TRUE);

	if ((GetWindowStyle(m_hCwnd) & WS_CHILD))
		::SetParent(m_hCwnd, m_hPwnd);
	else
	{
		::SetParent(m_hCwnd, ::GetDesktopWindow());
		::SetWindowLongPtr(m_hCwnd, GWLP_HWNDPARENT, LONG_PTR(m_hPwnd));
	}
}

void CWndExtBorderDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: 在此处添加消息处理程序代码

	if (::GetParent(m_hCwnd) == this->m_hWnd)
		::SetParent(m_hCwnd, this->m_hWnd);

	if (m_flock == false)
	{
		m_lRect.SetRect(0, 0, cx, cy);
	}

	::MoveWindow(m_hCwnd, m_lRect.left, m_lRect.top, m_lRect.Width(), m_lRect.Height(), TRUE);
}

void CWndExtBorderDlg::OnBnClickedButtonOkCancel()
{
}
/*
void CWndExtBorderDlg::OnCbtHook(WPARAM wParam, LPARAM lParam)
{
::CRect	cRect(0,0,0,0);
::GetClientRect(m_hWnd,&cRect);
::MoveWindow(m_hCwnd,0,0,cRect.Width(),cRect.Height(),TRUE);
}*/
void CWndExtBorderDlg::OnLockRect(LPRECT lpRect)
{
	m_lRect = lpRect[0];

	m_lRect.right -= m_lRect.left;
	m_lRect.bottom -= m_lRect.top;

	::ScreenToClient(m_hWnd, LPPOINT(&m_lRect));

	m_lRect.right += m_lRect.left;
	m_lRect.bottom += m_lRect.top;

	::MoveWindow(m_hCwnd, m_lRect.left, m_lRect.top, m_lRect.Width(), m_lRect.Height(), TRUE);

	if (m_flock == false)
	{
		m_flock = true;
		GetSystemMenu(FALSE)->AppendMenu(MF_STRING, IDM_SYS_SORPTION, TEXT("吸附边缘"));
	}

}
//If the normal calls, should return true
bool  CWndExtBorderDlg::OnCheckRLockWizardDlg()
{
	return true;
}
void CWndExtBorderDlg::OnTimer(UINT_PTR nIDEvent)
{
	__super::OnTimer(nIDEvent);
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == DT_LOCK)
	{
		::MoveWindow(m_hCwnd, m_lRect.left, m_lRect.top, m_lRect.Width(), m_lRect.Height(), TRUE);
	}
}
