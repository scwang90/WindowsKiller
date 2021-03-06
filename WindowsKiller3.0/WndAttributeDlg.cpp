// WndAttributeDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndAttributeDlg.h"
#include "afxdialogex.h"


// CWndAttributeDlg 对话框

#define	WM_CHECKWINDOW	(WM_USER+256)

static	ULONG    g_LockNumber = 0;
static	ULONG    g_IWndAttributeNumber = 0;
static	HMODULE	 g_hModule = ::GetModuleHandle(NULL/*TEXT("WndAttribute.dll")*/);

IMPLEMENT_DYNAMIC(CWndAttributeDlg, CDialog)

CWndAttributeDlg::CWndAttributeDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_WINATT_DIALOG, pParent)
{
	//////////////////////////////////////////////////////////////////////////////
	//	Unknown
	m_Ref = 0;
	g_IWndAttributeNumber++;
	//////////////////////////////////////////////////////////////////////////////
	//	IWndAttribute

	m_hIcon = AfxGetApp()->LoadIcon(128/*IDR_MAINFRAME*/);

	if (S_OK != DllWndEngine::DllCoCreateObject(CLSID_IWndEngine, IID_IWndEngine, (void**)&mPtrWndEngine))
	{
		AfxMessageBox(TEXT("Create CLSID_IWndEngine failed!\n"), MB_ICONERROR | MB_TOPMOST);
		ExitProcess(0);
	}
}

CWndAttributeDlg::~CWndAttributeDlg()
{
	mPtrWndEngine->Release();
}


HRESULT  CWndAttributeDlg::QueryInterface(const IID& iid, void **ppv)
{
	if (iid == IID_IUnknown)
	{
		*ppv = (IUnknown *)this;
		((IUnknown *)(*ppv))->AddRef();
	}
	else if (iid == IID_IWndAttribute)
	{
		*ppv = (IWndAttribute *)this;
		((IWndAttribute *)(*ppv))->AddRef();
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG	 CWndAttributeDlg::AddRef()
{
	m_Ref++;
	return  (ULONG)m_Ref;
}

ULONG	 CWndAttributeDlg::Release()
{
	m_Ref--;
	if (m_Ref == 0) {
		g_IWndAttributeNumber--;
		delete this;
		return 0;
	}
	return  (ULONG)m_Ref;
}
HWND CWndAttributeDlg::GetWndHandle()
{
	return m_hWnd;
}
bool CWndAttributeDlg::QueryWindowAttrib(HWND hWnd)
{
	if (!IsWindowVisible() || IsIconic())
		ShowWindow(SW_NORMAL);

	if (::IsWindow(hWnd) == TRUE)
	{
		m_hCWnd = hWnd;
		mPtrWndEngine->QueryWindow(hWnd);
		m_WndAttClassDlg.UpdateWindowInfo();
		m_WndAttGeneralDlg.UpdateWindowInfo();
		m_WndAttProcessDlg.UpdateWindowInfo();
		m_WndAttStylesDlg.UpdateWindowInfo();
		m_WndAttWindowDlg.UpdateWindowInfo();

		return true;
	}
	return false;
}
bool CWndAttributeDlg::CreateAttributeView(CWnd* pParentWnd)
{
	return Create(ATL_MAKEINTRESOURCE(CWndAttributeDlg::IDD), pParentWnd) != FALSE;
}
bool CWndAttributeDlg::SetIFileEngine(IFileEngine*pIFileEngine)
{
	return true;
}
BOOL CWndAttributeDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
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
void CWndAttributeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWndAttributeDlg)
	DDX_Control(pDX, IDC_TAB, m_ctrlTab);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWndAttributeDlg, CDialog)
	//{{AFX_MSG_MAP(CWndAttDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnSelchangeTab)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(WM_CHECKWINDOW, &CWndAttributeDlg::OnCheckWindow)
	ON_BN_CLICKED(IDREFRESH, &CWndAttributeDlg::OnBnClickedRefresh)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWndAttDlg message handlers

BOOL CWndAttributeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	// TODO: Add extra initialization here

	m_ctrlTab.InsertItem(0, TEXT("常规"));
	m_WndAttGeneralDlg.Create(ATL_MAKEINTRESOURCE(CWndAttGeneralDlg::IDD), &m_ctrlTab);
	m_WndAttGeneralDlg.SetIWndEngine(mPtrWndEngine);
	m_DlgArray.Add(&m_WndAttGeneralDlg);

	m_ctrlTab.InsertItem(1, TEXT("样式"));
	m_WndAttStylesDlg.Create(ATL_MAKEINTRESOURCE(CWndAttStylesDlg::IDD), &m_ctrlTab);
	m_WndAttStylesDlg.SetIWndEngine(mPtrWndEngine);
	m_DlgArray.Add(&m_WndAttStylesDlg);

	m_ctrlTab.InsertItem(2, TEXT("窗口"));
	m_WndAttWindowDlg.Create(ATL_MAKEINTRESOURCE(CWndAttWindowDlg::IDD), &m_ctrlTab);
	m_WndAttWindowDlg.SetIWndEngine(mPtrWndEngine);
	m_DlgArray.Add(&m_WndAttWindowDlg);

	m_ctrlTab.InsertItem(3, TEXT("类"));
	m_WndAttClassDlg.Create(ATL_MAKEINTRESOURCE(CWndAttClassDlg::IDD), &m_ctrlTab);
	m_WndAttClassDlg.SetIWndEngine(mPtrWndEngine);
	m_DlgArray.Add(&m_WndAttClassDlg);

	m_ctrlTab.InsertItem(4, TEXT("进程"));
	m_WndAttProcessDlg.Create(ATL_MAKEINTRESOURCE(CWndAttProcessDlg::IDD), &m_ctrlTab);
	m_WndAttProcessDlg.SetIWndEngine(mPtrWndEngine);
	m_DlgArray.Add(&m_WndAttProcessDlg);

	CRect rectTabClient;
	CRect rectTab;

	m_ctrlTab.GetClientRect(&rectTabClient);
	m_ctrlTab.GetItemRect(0, &rectTab);

	// Adjust rect for dialog placement
	rectTabClient.top += rectTab.Height() + 4;
	rectTabClient.left += 4;

	for (INT_PTR i = m_DlgArray.GetSize() - 1; i >= 0; i--)
	{
		CDialog* pDlg = (CDialog*)m_DlgArray.GetAt(i);

		pDlg->SetWindowPos(&CWnd::wndTopMost, rectTabClient.left - 1, rectTabClient.top, rectTabClient.Width() - 4, rectTabClient.Height() - 4, SWP_NOZORDER);
		pDlg->ShowWindow((i>0) ? SW_HIDE : SW_SHOW);
	}

	LOGFONT	LogFont = { -12,0,0,0,400,0,0,0,134,3,2,1,2,TEXT("宋体") };
	m_cFont.CreateFontIndirect(&LogFont);

	//SetChildFont(m_hWnd, LPARAM(&m_cFont));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWndAttributeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWndAttributeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWndAttributeDlg::OnQueryDragIcon()
{
	return (HCURSOR)m_hIcon;
}

void CWndAttributeDlg::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	CDialog* pDlg;

	m_ctrlTab.LockWindowUpdate();

	// Hide all tab dialogs
	for (int i = 0; i<m_DlgArray.GetSize(); i++)
	{
		pDlg = (CDialog*)m_DlgArray.GetAt(i);
		pDlg->ShowWindow(SW_HIDE);
	}

	// Show the selected tab dialog
	pDlg = (CDialog*)m_DlgArray.GetAt(m_ctrlTab.GetCurSel());
	pDlg->ShowWindow(SW_SHOW);

	m_ctrlTab.UnlockWindowUpdate();

	*pResult = 0;
}
void CWndAttributeDlg::OnClose()
{
	this->ShowWindow(SW_HIDE);
	this->GetParent()->SetActiveWindow();
}

void CWndAttributeDlg::OnCancel()
{
	this->ShowWindow(SW_HIDE);
	this->GetParent()->SetActiveWindow();
}

void CWndAttributeDlg::OnOK()
{
	this->ShowWindow(SW_HIDE);
	this->GetParent()->SetActiveWindow();
}
void CWndAttributeDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	// TODO: 在此处添加消息处理程序代码
	if (bShow)this->CenterWindow();
}
LRESULT CWndAttributeDlg::OnCheckWindow(WPARAM wParam, LPARAM lParam)
{
	return SetCheckWindow(HWND(lParam));
}
bool CWndAttributeDlg::SetCheckWindow(HWND hWnd)
{
	if (::IsWindow(hWnd) == TRUE)
	{
		m_hCWnd = hWnd;
		mPtrWndEngine->QueryWindow(hWnd);
		m_WndAttClassDlg.UpdateWindowInfo();
		m_WndAttGeneralDlg.UpdateWindowInfo();
		m_WndAttProcessDlg.UpdateWindowInfo();
		m_WndAttStylesDlg.UpdateWindowInfo();
		m_WndAttWindowDlg.UpdateWindowInfo();
		return true;
	}
	return false;
}
void CWndAttributeDlg::OnBnClickedRefresh()
{
	// TODO: 在此添加控件通知处理程序代码
	SetCheckWindow(m_hCWnd);
}
BOOL CALLBACK CWndAttributeDlg::SetChildFont(HWND hWnd, LPARAM lparam)
{
	::SendMessage(hWnd, WM_SETFONT, WPARAM(((CFont*)lparam)->GetSafeHandle()), 1);
	EnumChildWindows(hWnd, SetChildFont, lparam);
	return TRUE;
}
