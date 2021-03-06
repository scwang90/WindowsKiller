// WndWizFindOptionDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndWizFindOptionDlg.h"
#include "afxdialogex.h"


// CWndWizFindOptionDlg 对话框

IMPLEMENT_DYNAMIC(CWndWizFindOptionDlg, CDialog)

CWndWizFindOptionDlg::CWndWizFindOptionDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_FINDOPTION, pParent)
{
	m_uFlag = 0xFFFFFFFF;
	m_hIcon = AfxGetApp()->LoadIcon(128/*IDR_MAINFRAME*/);
}

CWndWizFindOptionDlg::~CWndWizFindOptionDlg()
{
}

BOOL CWndWizFindOptionDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	ASSERT(pParentWnd != NULL);
	ASSERT(IS_INTRESOURCE(lpszTemplateName) || AfxIsValidString(lpszTemplateName));

	m_lpszTemplateName = lpszTemplateName;  // used for help
	if (IS_INTRESOURCE(m_lpszTemplateName) && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD_PTR)m_lpszTemplateName);

	HINSTANCE hInst = ::GetModuleHandle(NULL/*TEXT("WndWizard.dll")*/);
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);

	BOOL bResult = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);
	UnlockResource(hTemplate);
	FreeResource(hTemplate);

	return bResult;
}
void CWndWizFindOptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWndWizFindOptionDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDCANCEL, &CWndWizFindOptionDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDOK, &CWndWizFindOptionDlg::OnBnClickedButtonOk)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CWndWizFindOptionDlg 消息处理程序

BOOL CWndWizFindOptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

									// TODO: 在此添加额外的初始化代码
	::EnableWindow(GetDlgItem(IDC_CHECK_INVISIBLE)->m_hWnd, TRUE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CWndWizFindOptionDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}
BOOL CWndWizFindOptionDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int	 nID = LOWORD(wParam);
	HWND hCtl = HWND(lParam);
	UINT code = UINT(HIWORD(wParam));

	if (code == BN_CLICKED)
	{
		if (nID == IDC_RADIO_SUPPER || nID == IDC_RADIO_NORMAL)
		{
			::EnableWindow(GetDlgItem(IDC_CHECK_INVISIBLE)->m_hWnd, nID == IDC_RADIO_SUPPER);
			::EnableWindow(GetDlgItem(IDC_CHECK_DISABLED)->m_hWnd, nID == IDC_RADIO_SUPPER);
			::EnableWindow(GetDlgItem(IDC_CHECK_TRANSPARENT)->m_hWnd, nID == IDC_RADIO_SUPPER);
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}
// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWndWizFindOptionDlg::OnPaint()
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
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CWndWizFindOptionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CWndWizFindOptionDlg::OnClose()
{
	CDialog::OnClose();
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	this->SetFindOption(m_uFlag);
	this->OnBnClickedButtonOkCancel();
}

void CWndWizFindOptionDlg::OnBnClickedButtonOkCancel()
{
	this->ShowWindow(SW_HIDE);
	this->GetParent()->EnableWindow(true);
	this->GetParent()->SetActiveWindow();
}
void CWndWizFindOptionDlg::OnBnClickedButtonOk()
{
	if (this->IsDlgButtonChecked(IDC_RADIO_NORMAL))
		m_uFlag = 0xFFFFFFFF;
	else
	{
		m_uFlag = 0;

		if (this->IsDlgButtonChecked(IDC_CHECK_INVISIBLE))
			m_uFlag |= CWP_SKIPINVISIBLE;

		if (this->IsDlgButtonChecked(IDC_CHECK_DISABLED))
			m_uFlag |= CWP_SKIPDISABLED;

		if (this->IsDlgButtonChecked(IDC_CHECK_TRANSPARENT))
			m_uFlag |= CWP_SKIPTRANSPARENT;

	}
#ifdef _DEBUG
	printf("CWndWizFindOptionDlg::flags = %d\n", m_uFlag);
#endif	

	this->OnBnClickedButtonOkCancel();
}
void CWndWizFindOptionDlg::OnBnClickedButtonCancel()
{
	this->SetFindOption(m_uFlag);
	this->OnBnClickedButtonOkCancel();
}
void CWndWizFindOptionDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	// TODO: 在此处添加消息处理程序代码
	this->GetParent()->EnableWindow(false);
	if (bShow)this->CenterWindow();
}
void CWndWizFindOptionDlg::SetFindOption(UINT flag)
{
	m_uFlag = flag;
	if (0xFFFFFFFF == flag)
	{
		this->CheckDlgButton(IDC_RADIO_NORMAL, TRUE);
		this->CheckDlgButton(IDC_RADIO_SUPPER, FALSE);
		::EnableWindow(GetDlgItem(IDC_CHECK_INVISIBLE)->m_hWnd, FALSE);
		::EnableWindow(GetDlgItem(IDC_CHECK_DISABLED)->m_hWnd, FALSE);
		::EnableWindow(GetDlgItem(IDC_CHECK_TRANSPARENT)->m_hWnd, FALSE);
		this->CheckDlgButton(IDC_CHECK_INVISIBLE, TRUE);
	}
	else
	{
		this->CheckDlgButton(IDC_RADIO_NORMAL, FALSE);
		this->CheckDlgButton(IDC_RADIO_SUPPER, TRUE);
		this->CheckDlgButton(IDC_CHECK_INVISIBLE, (flag & CWP_SKIPINVISIBLE) != 0);
		this->CheckDlgButton(IDC_CHECK_DISABLED, (flag & CWP_SKIPDISABLED) != 0);
		this->CheckDlgButton(IDC_CHECK_TRANSPARENT, (flag & CWP_SKIPTRANSPARENT) != 0);
		::EnableWindow(GetDlgItem(IDC_CHECK_INVISIBLE)->m_hWnd, TRUE);
		::EnableWindow(GetDlgItem(IDC_CHECK_DISABLED)->m_hWnd, TRUE);
		::EnableWindow(GetDlgItem(IDC_CHECK_TRANSPARENT)->m_hWnd, TRUE);
	}
}
