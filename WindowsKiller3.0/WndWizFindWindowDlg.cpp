// WinWizardFindWindowDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndWizFindWindowDlg.h"
#include "afxdialogex.h"


// CWndWizFindWindowDlg 对话框

IMPLEMENT_DYNAMIC(CWndWizFindWindowDlg, CDialog)

CWndWizFindWindowDlg::CWndWizFindWindowDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_FINDWINDOW, pParent)
{
	m_hFwnd = NULL;
	m_pRFindWindow = NULL;

	HRESULT 		hResult = NULL;
	IUnknown *		pUnknown = NULL;

	m_hIcon = AfxGetApp()->LoadIcon(128/*IDR_MAINFRAME*/);

	hResult = DllWndEngine::DllCoCreateObject(CLSID_IWndEngine, IID_IUnknown, (void**)&pUnknown);
	if (hResult != S_OK)
	{
		AfxMessageBox(TEXT("Create CLSID_IWndEngine failed!\n"), MB_ICONERROR | MB_TOPMOST);
		ExitProcess(0);
	}
	hResult = pUnknown->QueryInterface(IID_IWndEngine, (void **)&mPtrWndEngine);
	if (hResult != S_OK) {
		pUnknown->Release();
		AfxMessageBox(TEXT("QueryInterface IWndEngine failed!\n"), MB_ICONERROR | MB_TOPMOST);
		ExitProcess(0);
	}
	pUnknown->Release();
}

CWndWizFindWindowDlg::~CWndWizFindWindowDlg()
{
	mPtrWndEngine->Release();
}

BOOL CWndWizFindWindowDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
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
void CWndWizFindWindowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWndWizFindWindowDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDCANCEL, &CWndWizFindWindowDlg::OnBnClickedButtonOkCancel)
	ON_BN_CLICKED(IDOK, &CWndWizFindWindowDlg::OnBnClickedButtonOkCancel)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON_FIND, &CWndWizFindWindowDlg::OnBnClickedButtonFind)
	ON_BN_CLICKED(IDC_CHECK_FUZZY, &CWndWizFindWindowDlg::OnBnClickedCheckFuzzy)
	ON_BN_CLICKED(IDC_CHECK_CASEMATTER, &CWndWizFindWindowDlg::OnBnClickedCheckCasematter)
END_MESSAGE_MAP()


// CWndWizFindWindowDlg 消息处理程序

BOOL CWndWizFindWindowDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

									// TODO: 在此添加额外的初始化代码
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_FIND), FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CWndWizFindWindowDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}
BOOL CWndWizFindWindowDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int	 nID = LOWORD(wParam);
	HWND hCtl = HWND(lParam);
	UINT code = UINT(HIWORD(wParam));

	if (code == EN_CHANGE)
	{
		if (nID == IDC_EDIT_CLASS || nID == IDC_EDIT_TITLE)
		{
			static	TCHAR	szClass[128] = TEXT("");
			static	TCHAR	szTitle[128] = TEXT("");
			this->GetDlgItemText(IDC_EDIT_CLASS, szClass, 128);
			this->GetDlgItemText(IDC_EDIT_TITLE, szTitle, 128);

			::EnableWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_FIND), szClass[0] || szTitle[0]);

			if (m_hFwnd != NULL)
			{
				m_hFwnd = NULL;
				this->SetDlgItemText(IDC_BUTTON_FIND, TEXT("查找"));
			}
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}
// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWndWizFindWindowDlg::OnPaint()
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
HCURSOR CWndWizFindWindowDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CWndWizFindWindowDlg::OnClose()
{
	CDialog::OnClose();
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ASSERT(m_pRFindWindow != NULL);
	this->ShowWindow(SW_HIDE);
	this->GetParent()->SetActiveWindow();
	this->m_pRFindWindow->OnEndFindWindow();
}
void CWndWizFindWindowDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: 在此处添加消息处理程序代码
}

void CWndWizFindWindowDlg::OnBnClickedButtonOkCancel()
{
	ASSERT(m_pRFindWindow != NULL);
	this->ShowWindow(SW_HIDE);
	this->GetParent()->SetActiveWindow();
	this->m_pRFindWindow->OnEndFindWindow();
}
void CWndWizFindWindowDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	// TODO: 在此处添加消息处理程序代码
	if (bShow)this->CenterWindow();
}

void CWndWizFindWindowDlg::OnBnClickedButtonFind()
{
	// TODO: 在此添加控件通知处理程序代码
	ASSERT(m_pRFindWindow != NULL);

	TCHAR	szClass[128] = TEXT("");
	TCHAR	szTitle[128] = TEXT("");
	this->GetDlgItemText(IDC_EDIT_CLASS, szClass, 128);
	this->GetDlgItemText(IDC_EDIT_TITLE, szTitle, 128);

	LPCTSTR	lpClass = szClass[0] ? szClass : NULL;
	LPCTSTR	lpTitle = szTitle[0] ? szTitle : NULL;

	bool	bFuzzy = IsDlgButtonChecked(IDC_CHECK_FUZZY) != 0;
	bool	bCaseMatters = IsDlgButtonChecked(IDC_CHECK_CASEMATTER) != 0;
	HWND	hFwnd = mPtrWndEngine->FindWindowEx(NULL, m_hFwnd, lpClass, lpTitle, bCaseMatters, bFuzzy);

	if (hFwnd != NULL)
	{
		m_hFwnd = hFwnd;
		this->SetDlgItemText(IDC_BUTTON_FIND, TEXT("下一个"));
		m_pRFindWindow->OnFindWindow(m_hFwnd);
	}
	else
	{
		this->SetDlgItemText(IDC_BUTTON_FIND, TEXT("查找"));
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_FIND), FALSE);
		if (m_hFwnd != NULL)
			MessageBox(TEXT("查找结束。"), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
		else
			MessageBox(TEXT("窗口未找到！"), TEXT("错误"), MB_TOPMOST | MB_ICONWARNING);
	}
}
bool CWndWizFindWindowDlg::SetRFindWindow(RFindWindow* pRFindWindow)
{
#ifdef _DEBUG
	ASSERT(pRFindWindow != NULL);
#endif
	if (pRFindWindow->OnCheckRFindWindow())
	{
		m_pRFindWindow = pRFindWindow;
		return true;
	}
	return false;
}
void CWndWizFindWindowDlg::SetFindParam(LPCTSTR lpClass, LPCTSTR lpTitle, HWND hwnd)
{
	this->SetDlgItemText(IDC_EDIT_CLASS, lpClass);
	this->SetDlgItemText(IDC_EDIT_TITLE, lpTitle);
	if (::IsWindow(hwnd))
	{
		m_hFwnd = hwnd;
		this->SetDlgItemText(IDC_BUTTON_FIND, TEXT("下一个"));
	}
}

void CWndWizFindWindowDlg::OnBnClickedCheckFuzzy()
{
	// TODO: 在此添加控件通知处理程序代码
	static	TCHAR	szClass[128] = TEXT("");
	static	TCHAR	szTitle[128] = TEXT("");
	this->GetDlgItemText(IDC_EDIT_CLASS, szClass, 128);
	this->GetDlgItemText(IDC_EDIT_TITLE, szTitle, 128);

	::EnableWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_FIND), szClass[0] || szTitle[0]);

	if (m_hFwnd != NULL)
	{
		m_hFwnd = NULL;
		this->SetDlgItemText(IDC_BUTTON_FIND, TEXT("查找"));
	}
}


void CWndWizFindWindowDlg::OnBnClickedCheckCasematter()
{
	// TODO: 在此添加控件通知处理程序代码
	static	TCHAR	szClass[128] = TEXT("");
	static	TCHAR	szTitle[128] = TEXT("");
	this->GetDlgItemText(IDC_EDIT_CLASS, szClass, 128);
	this->GetDlgItemText(IDC_EDIT_TITLE, szTitle, 128);

	::EnableWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_FIND), szClass[0] || szTitle[0]);

	if (m_hFwnd != NULL)
	{
		m_hFwnd = NULL;
		this->SetDlgItemText(IDC_BUTTON_FIND, TEXT("查找"));
	}
}
