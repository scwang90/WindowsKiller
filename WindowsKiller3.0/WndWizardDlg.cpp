// WndWizardDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndWizardDlg.h"
#include "WndWizHotkeyDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDT_FIND		100
#define IDT_CHECK		101
//#define IDM_SETFONT		101
//#define IDM_SETCOLOR	102

#define	FV_WIZARD_BOOL_EMPTY	TEXT("Wizard_bIsEmpty")
#define	FV_WIZARD_BOOL_TOPMOST	TEXT("Wizard_bIsTopMost")
#define	FV_WIZARD_BOOL_ENGARY	TEXT("Wizard_bIsEnGary")
#define	FV_WIZARD_BOOL_HIDEMODE	TEXT("Wizard_bIsHideMode")
#define	FV_WIZARD_LONG_COLOR	TEXT("Wizard_lSignColor")
#define	FV_WIZARD_STRUCT_FONT	TEXT("Wizard_StructFont")


#define FM_NOTIFYMSG	(WM_USER+100)
#define HT_ENDAUTOMODE		1000
#define HT_OPENMAINPANEL	1001
#define HT_AUTOMODEBEGIN	1002
#define HT_AUTOMODEEND		1003
#define HT_FINDCPUPAGE		1004

// CWndWizardDlg 对话框

ULONG    g_WndWizardDlgNumber = 0;


IWndPreview*	CWndWizardDlg::m_pIWndPreview = NULL;
IWndModule*		CWndWizardDlg::m_pIWndModule = NULL;
IFileEngine*	CWndWizardDlg::m_pIFileEngine = NULL;
IProcEngine*	CWndWizardDlg::m_pIProcEngine = NULL;


IMPLEMENT_DYNAMIC(CWndWizardDlg, CDialog)

CWndWizardDlg::CWndWizardDlg(IWndWizard* pIViewWizard, bool isBack, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_WIZARD_DIALOG, pParent)
{

	g_WndWizardDlgNumber++;

	this->m_nShow = 0;
	this->m_Style = STYLE_NULL;
	this->m_dwPreModulePid = 0;
	this->m_bIsBackstage = isBack;
	this->m_pIViewWizard = pIViewWizard;

	m_wMainPanel1 = MOD_SHIFT;
	m_wMainPanel2 = VK_F1;
	m_wBackBegin1 = MOD_SHIFT;
	m_wBackBegin2 = VK_F2;
	m_wBackEnd1 = MOD_SHIFT;
	m_wBackEnd2 = VK_F3;
	m_wFindCpu1 = MOD_SHIFT;
	m_wFindCpu2 = VK_F4;
	m_wEndFind1 = MOD_SHIFT;
	m_wEndFind2 = VK_ESCAPE;

	m_bIsTopMost = true;
	m_bIsEnGary = false;
	m_bIsHideMode = false;
	m_lSignColor = RGB(255, 0, 0);

	m_hIcon = AfxGetApp()->LoadIcon(128/*IDR_MAINFRAME*/);

	try {
		if (FAILED(DllWndEngine::DllCoCreateObject(CLSID_IWndEngine, IID_IWndEngine, (void**)&mPtrWndEngine)))
			throw TEXT("Create CLSID_IFileEngine failed!\n");

		if (FAILED(DllWndSigned::DllCoCreateObject(CLSID_IWndSigned, IID_IWndSigned, (void**)&m_pIWndSigned)))
			throw TEXT("Create CLSID_WndSigned failed!\n");

		if (FAILED(CWindowsKiller30App::DllCoCreateObject(CLSID_IWndExtract, IID_IWndExtract, (void**)&m_pIWndExtract)))
			throw TEXT("Create CLSID_IWndExtract failed!\n");

		if (FAILED(CWindowsKiller30App::DllCoCreateObject(CLSID_IWndAttribute, IID_IWndAttribute, (void**)&m_pIWndAttrib)))
			throw TEXT("Create CLSID_IWndAttribute failed!\n");
	}
	catch (LPCTSTR lpError) {
		AfxMessageBox(lpError, MB_ICONERROR | MB_TOPMOST);
		ExitProcess(0);
	}

	mHcurEmpty = AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_EMPTY));
	mHcurFinder = AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_FINDER));
	mHcurHander = AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_HANDER));

	//m_curEmpty = (HCURSOR)LoadImage(NULL, TEXT("image/curnull.cur"), IMAGE_CURSOR, 0/*int cxDesired*/, 0/*int CyDesired*/, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	//if (m_curEmpty == NULL)MessageBox(GetLastErrorInfo(TEXT("image/curnull.cur 光标文件加载失败")));

	//mHcurFinder = (HCURSOR)LoadImage(NULL, TEXT("image/arrowcop.cur"), IMAGE_CURSOR, 0/*int cxDesired*/, 0/*int CyDesired*/, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	//if (mHcurFinder == NULL)MessageBox(GetLastErrorInfo(TEXT("image/arrowcop.cur 光标文件加载失败")));

	//mHcurHander = (HCURSOR)LoadImage(NULL, TEXT("image/curhand.cur"), IMAGE_CURSOR, 0/*int cxDesired*/, 0/*int CyDesired*/, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	//if (mHcurHander == NULL)MessageBox(GetLastErrorInfo(TEXT("image/curhand.cur 光标文件加载失败")));

	CWndWizardDlg::Create(ATL_MAKEINTRESOURCE(CWndWizardDlg::IDD), pParent);

	if (this->m_bIsBackstage == TRUE) {
		if (RegisterHotKey(m_hWnd, HT_OPENMAINPANEL, m_wMainPanel1, m_wMainPanel2) == FALSE)
		{
			mCtlHotKey.SetHotKey(m_wMainPanel2, m_wMainPanel1);
			AfxMessageBox(CString(TEXT("注册快捷键【")) + mCtlHotKey.GetHotKeyName() + TEXT("】失败！原因可能是快捷键与别的程序冲突，请更新设置快捷键"));
		}
		if (RegisterHotKey(m_hWnd, HT_AUTOMODEBEGIN, m_wBackBegin1, m_wBackBegin2) == FALSE)
		{
			mCtlHotKey.SetHotKey(m_wBackBegin2, m_wBackBegin1);
			AfxMessageBox(CString(TEXT("注册快捷键【")) + mCtlHotKey.GetHotKeyName() + TEXT("】失败！原因可能是快捷键与别的程序冲突，请更新设置快捷键"));
		}
		if (RegisterHotKey(m_hWnd, HT_AUTOMODEEND, m_wBackEnd1, m_wBackEnd2) == FALSE)
		{
			mCtlHotKey.SetHotKey(m_wBackEnd2, m_wBackEnd1);
			AfxMessageBox(CString(TEXT("注册快捷键【")) + mCtlHotKey.GetHotKeyName() + TEXT("】失败！原因可能是快捷键与别的程序冲突，请更新设置快捷键"));
		}
		if (RegisterHotKey(m_hWnd, HT_FINDCPUPAGE, m_wFindCpu1, m_wFindCpu2) == FALSE)
		{
			mCtlHotKey.SetHotKey(m_wFindCpu2, m_wFindCpu1);
			AfxMessageBox(CString(TEXT("注册快捷键【")) + mCtlHotKey.GetHotKeyName() + TEXT("】失败！原因可能是快捷键与别的程序冲突，请更新设置快捷键"));
		}
	}
	else {
		CDialog::ShowWindow(SW_NORMAL);
	}

	this->SetTimer(IDT_CHECK, 1000, NULL);
}

CWndWizardDlg::~CWndWizardDlg()
{
	DestroyCursor(mHcurFinder);
	DestroyCursor(mHcurHander);

	mPtrWndEngine->Release();
	m_pIWndSigned->Release();
	m_pIWndExtract->Release();

	g_WndWizardDlgNumber--;
}

BOOL CWndWizardDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
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
void CWndWizardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FIND, mCurFind);
	DDX_Control(pDX, IDC_MOVE, mCurMove);
	DDX_Control(pDX, IDC_HOTKEYCTRL, mCtlHotKey);
}

BEGIN_MESSAGE_MAP(CWndWizardDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_STN_CLICKED(IDC_FIND, &CWndWizardDlg::OnStnClickedFind)
	ON_STN_CLICKED(IDC_MOVE, &CWndWizardDlg::OnStnClickedMove)
	ON_BN_CLICKED(IDC_TOPMOST, &CWndWizardDlg::OnBnClickedTopmost)
	ON_WM_LBUTTONUP()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDCANCEL, &CWndWizardDlg::OnBnClickedButtonOkCancel)
	ON_BN_CLICKED(IDOK, &CWndWizardDlg::OnBnClickedButtonOkCancel)
	ON_BN_CLICKED(IDC_BUTTON_CHANGLE, &CWndWizardDlg::OnBnClickedButtonChangle)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CWndWizardDlg::OnBnClickedButtonUpdate)
	ON_BN_CLICKED(IDC_BUTTON_TOPARENT, &CWndWizardDlg::OnBnClickedButtonToparent)
	ON_WM_KILLFOCUS()
	ON_WM_ACTIVATE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_AUTOMODE, &CWndWizardDlg::OnBnClickedAutoMode)
	ON_BN_CLICKED(IDC_BUTTON_PICKUP, &CWndWizardDlg::OnBnClickedButtonPickup)
	ON_BN_CLICKED(IDC_BUTTON_FINDWINDOW, &CWndWizardDlg::OnBnClickedButtonFindwindow)
	ON_WM_QUERYOPEN()
	ON_BN_CLICKED(IDC_GRAY, &CWndWizardDlg::OnBnClickedGray)
	ON_BN_CLICKED(IDC_HIDEMODE, &CWndWizardDlg::OnBnClickedHidemode)
	ON_MESSAGE(WM_HOTKEY, CWndWizardDlg::OnHotKey)
	//ON_BN_CLICKED(IDC_BUTTON_FINDMODE, &CWndWizardDlg::OnBnClickedButtonFindmode)
	ON_COMMAND(ID_SET_HOTKEY, &CWndWizardDlg::OnSetHotkey)
	ON_BN_CLICKED(IDC_BUTTON_SENDMESAGE, &CWndWizardDlg::OnBnClickedButtonSendmesage)
END_MESSAGE_MAP()


// CWndWizardDlg 消息处理程序

BOOL CWndWizardDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	m_FindOptionDlg.Create(ATL_MAKEINTRESOURCE(CWndWizFindOptionDlg::IDD), this);
	m_FindOptionDlg.SetFindOption(0xFFFFFFFF);
	m_FindWindowDlg.Create(ATL_MAKEINTRESOURCE(CWndWizFindWindowDlg::IDD), this);
	m_FindWindowDlg.SetRFindWindow(this);

	m_pIWndAttrib->CreateAttributeView(this);
	mCurFind.SetCursor(mHcurFinder);
	mCurMove.SetCursor(mHcurHander);

	//LOGFONT	LogFont = { -10,0,0,0,400,0,0,0,0,3,2,1,18,TEXT("Palatino Linotype") };
	LOGFONT	LogFont = { -12,0,0,0,400,0,0,0,134,3,2,1,2,TEXT("宋体") };
	GetFont()->GetLogFont(&LogFont);
#ifdef DEBUG
	_tprintf(TEXT("LOGFONT = %s\n"), LogFont.lfFaceName);
#endif // DEBUG

	if (m_pIFileEngine != NULL)
	{
		bool bIsEmpty = true;
		m_pIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_WIZARD_BOOL_EMPTY, &bIsEmpty);
		if (bIsEmpty == true)
		{
			TYPEDATA cTypeData[] = {
			{ IFileEngine::VT_BOOL,FV_WIZARD_BOOL_EMPTY,0,TD_VALUE },
			{ IFileEngine::VT_BOOL,FV_WIZARD_BOOL_TOPMOST,LPCVOID(true),TD_VALUE },
			{ IFileEngine::VT_BOOL,FV_WIZARD_BOOL_ENGARY,0,TD_VALUE },
			{ IFileEngine::VT_BOOL,FV_WIZARD_BOOL_HIDEMODE,0,TD_VALUE },
			{ IFileEngine::VT_WORD,TEXT("MainPanel1"),LPCVOID(MOD_SHIFT),TD_VALUE },
			{ IFileEngine::VT_WORD,TEXT("MainPanel2"),LPCVOID(VK_F1),TD_VALUE },
			{ IFileEngine::VT_WORD,TEXT("BackBegin1"),LPCVOID(MOD_SHIFT),TD_VALUE },
			{ IFileEngine::VT_WORD,TEXT("BackBegin2"),LPCVOID(VK_F2),TD_VALUE },
			{ IFileEngine::VT_WORD,TEXT("BackEnd1"),LPCVOID(MOD_SHIFT),TD_VALUE },
			{ IFileEngine::VT_WORD,TEXT("BackEnd2"),LPCVOID(VK_F3),TD_VALUE },
			{ IFileEngine::VT_WORD,TEXT("FindCpu1"),LPCVOID(MOD_SHIFT),TD_VALUE },
			{ IFileEngine::VT_WORD,TEXT("FindCpu2"),LPCVOID(VK_F4),TD_VALUE },
			{ IFileEngine::VT_WORD,TEXT("EndFind1"),LPCVOID(MOD_SHIFT),TD_VALUE },
			{ IFileEngine::VT_WORD,TEXT("EndFind2"),LPCVOID(VK_ESCAPE),TD_VALUE },
			{ IFileEngine::VT_LONG,FV_WIZARD_LONG_COLOR,LPCVOID(RGB(255,0,0)),TD_VALUE },
			{ IFileEngine::VT_STRUCT,FV_WIZARD_STRUCT_FONT,&LogFont,sizeof(LOGFONT) },
			};
			m_pIFileEngine->AddVarValue(cTypeData, sizeof(cTypeData) / sizeof(TYPEDATA));

#ifdef	_DEBUG
			_tprintf(TEXT("CWndWizardDlg::IFileEngine::bIsEmpty == true\n"));
			m_pIFileEngine->OutPutDataInfo();
#endif
		}

		m_pIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_WIZARD_BOOL_TOPMOST, &m_bIsTopMost);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_WIZARD_BOOL_ENGARY, &m_bIsEnGary);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_WIZARD_BOOL_HIDEMODE, &m_bIsHideMode);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_LONG, FV_WIZARD_LONG_COLOR, &m_lSignColor);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_STRUCT, FV_WIZARD_STRUCT_FONT, &LogFont);
		//m_pIFileEngine->GetVarValue(IFileEngine::VT_WORD, TEXT("MainPanel1"), &m_wMainPanel1);
		//m_pIFileEngine->GetVarValue(IFileEngine::VT_WORD, TEXT("MainPanel2"), &m_wMainPanel2);
		//m_pIFileEngine->GetVarValue(IFileEngine::VT_WORD, TEXT("BackBegin1"), &m_wBackBegin1);
		//m_pIFileEngine->GetVarValue(IFileEngine::VT_WORD, TEXT("BackBegin2"), &m_wBackBegin2);
		//m_pIFileEngine->GetVarValue(IFileEngine::VT_WORD, TEXT("BackEnd1"), &m_wBackEnd1);
		//m_pIFileEngine->GetVarValue(IFileEngine::VT_WORD, TEXT("BackEnd2"), &m_wBackEnd2);
		//m_pIFileEngine->GetVarValue(IFileEngine::VT_WORD, TEXT("EndFind1"), &m_wEndFind1);
		//m_pIFileEngine->GetVarValue(IFileEngine::VT_WORD, TEXT("EndFind2"), &m_wEndFind2);
		//m_pIFileEngine->GetVarValue(IFileEngine::VT_WORD, TEXT("FindCpu1"), &m_wFindCpu1);
		//m_pIFileEngine->GetVarValue(IFileEngine::VT_WORD, TEXT("FindCpu2"), &m_wFindCpu2);
	}

	m_pIWndSigned->SetSignColor(m_lSignColor);
	if (m_bIsTopMost == true)
	{
		this->CheckDlgButton(IDC_TOPMOST, TRUE);
		FORWARD_WM_COMMAND(m_hWnd, IDC_TOPMOST, GetDlgItem(IDC_TOPMOST)->m_hWnd, 0, ::PostMessage);
	}
	if (m_bIsEnGary == true)
	{
		this->CheckDlgButton(IDC_GRAY, TRUE);
		m_FindOptionDlg.SetFindOption(1);
	}
	if (m_bIsHideMode == true)
	{
		this->CheckDlgButton(IDC_HIDEMODE, TRUE);
	}
	this->UpDateUserInterface();

#ifdef DEBUG
	_tprintf(TEXT("SetChildFont = %s\n"), LogFont.lfFaceName);
#endif // DEBUG

	m_cFont.CreateFontIndirect(&LogFont);
	EnumChildWindows(m_hWnd, SetChildFont, LPARAM(&m_cFont));

	CRect cRect;
	GetWindowRect(&cRect);

	POINT point = {
		LONG(GetSystemMetrics(SM_CXSCREEN) / 2 - (cRect.Width()) / 2 + g_WndWizardDlgNumber * 24),
		LONG(GetSystemMetrics(SM_CYSCREEN) / 2 - (cRect.Height()) / 2 + g_WndWizardDlgNumber * 24)
	};

	point.x %= (GetSystemMetrics(SM_CXSCREEN) - 30);
	point.y %= (GetSystemMetrics(SM_CYSCREEN) - 50);

	this->MoveWindow(point.x, point.y, cRect.Width(), cRect.Height());

	m_ToolTip.Create(this);
	m_ToolTip.SetDelayTime(100);
	m_ToolTip.SetMaxTipWidth(200);

	m_ToolTip.AddTool(this, TEXT("窗口杀手"), &cRect, 1);

	if (mMfcMenuBar.Create(this)) {
		mMfcMenuBar.SetPaneStyle(mMfcMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);
		// 防止菜单栏在激活时获得焦点
		CMFCPopupMenu::SetForceMenuFocus(FALSE);
		//使菜单栏可停靠
		//EnableDocking(CBRS_ALIGN_ANY);
		//DockPane(&mMfcMenuBar);
	}
	else {
		AfxMessageBox(TEXT("未能创建菜单栏"));
	}

	/*
	SetWindowLong(m_ToolTip,GWL_EXSTYLE,m_ToolTip.GetExStyle()&~WS_EX_LAYERED);
	*/
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CWndWizardDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_MINIMIZE && g_WndWizardDlgNumber == 1)
	{
		HWND hTrayWnd = ::FindWindow(TEXT("Shell_TrayWnd"), NULL);  //windows底部信息提示窗口
		HWND hNotifyWnd = ::FindWindowEx(hTrayWnd, NULL, TEXT("TrayNotifyWnd"), NULL);
		if (hTrayWnd && hNotifyWnd)
		{
			CRect rtNotify;
			::GetWindowRect(m_hWnd, &m_wRect);
			::GetWindowRect(hNotifyWnd, &rtNotify);
			rtNotify.right -= 25;
			rtNotify.bottom -= 5;
			::DrawAnimatedRects(m_hWnd, IDANI_CAPTION, &m_wRect, &rtNotify);
		}
		CDialog::ShowWindow(SW_HIDE);
		CDialog::ShowWindow(SW_MINIMIZE);
		CDialog::ShowWindow(SW_HIDE);
		return;
		//CDialog::OnSysCommand(nID, lParam);

	}

	CDialog::OnSysCommand(nID, lParam);
}
BOOL CWndWizardDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int nID = LOWORD(wParam);
	HWND hCtl = HWND(lParam);
	UINT code = UINT(HIWORD(wParam));

	if (code == EN_KILLFOCUS)
	{
		switch (nID)
		{
		case IDC_EDIT_RIGHT:case IDC_EDIT_BOTTOM:
		case IDC_EDIT_WIDTH:case IDC_EDIT_HEIGHT:
		case IDC_EDIT_LEFT:case IDC_EDIT_TOP:
		{
			TCHAR	szTemp[32] = TEXT("");
			this->GetDlgItemText(nID, szTemp, 32);

			if (szTemp[0] == '\0')
			{
				szTemp[0] = '0';
				szTemp[1] = '\0';
				this->SetDlgItemText(nID, szTemp);
			}
		}
		}
	}
	else if (code == EN_SETFOCUS)
	{
		TCHAR	szClassName[128]= TEXT("");
		GetClassName(hCtl, szClassName, 128);
		if (lstrcmpi(szClassName, TEXT("edit")) == 0)
		{
			CEdit	cEdit;
			cEdit.Attach(hCtl);
			cEdit.SetSel(0, -1);
			cEdit.Detach();
		}
	}
	else if (code == EN_CHANGE)
	{
		int iCtl1 = 0, iCtl2 = 0, iCtl3 = 0;
		if (nID == IDC_EDIT_RIGHT || nID == IDC_EDIT_BOTTOM
			|| nID == IDC_EDIT_WIDTH || nID == IDC_EDIT_HEIGHT
			|| nID == IDC_EDIT_LEFT || nID == IDC_EDIT_TOP)
		{
			if (::GetFocus() != GetDlgItem(nID)->m_hWnd)
				return CWnd::OnCommand(wParam, lParam);
			switch (nID)
			{
			case IDC_EDIT_LEFT:
			case IDC_EDIT_WIDTH:
				iCtl1 = IDC_EDIT_LEFT; iCtl2 = IDC_EDIT_WIDTH; iCtl3 = IDC_EDIT_RIGHT;
				break;
			case IDC_EDIT_TOP:
			case IDC_EDIT_HEIGHT:
				iCtl1 = IDC_EDIT_TOP; iCtl2 = IDC_EDIT_HEIGHT; iCtl3 = IDC_EDIT_BOTTOM;
				break;
			case IDC_EDIT_RIGHT:
				iCtl1 = IDC_EDIT_LEFT; iCtl2 = IDC_EDIT_RIGHT; iCtl3 = IDC_EDIT_WIDTH;
				break;
			case IDC_EDIT_BOTTOM:
				iCtl1 = IDC_EDIT_TOP; iCtl2 = IDC_EDIT_BOTTOM; iCtl3 = IDC_EDIT_HEIGHT;
				break;
			}

			TCHAR	szTemp[32]= TEXT("");
			this->GetDlgItemText(iCtl1, szTemp, 32);
			int iCtl1 = _tcstol(szTemp, NULL, 0);
			this->GetDlgItemText(iCtl2, szTemp, 32);
			int iCtl2 = _tcstol(szTemp, NULL, 0);

			StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%d"), (iCtl3 == IDC_EDIT_HEIGHT || iCtl3 == IDC_EDIT_WIDTH) ? (iCtl2 - iCtl1) : (iCtl2 + iCtl1));
			this->SetDlgItemText(iCtl3, szTemp);
		}
	}
	else if (nID == ID_NEW_WINDOW)
	{
		this->m_pIViewWizard->LaunchViewWizard();
	}
	else if (nID == ID_FIND_WINDOW)
	{
		this->OnBnClickedButtonFindwindow();
	}
	else if (nID == ID_ENUM_CHILD)
	{
		HWND hCWnd = mPtrWndEngine->GetWindow();
		HWND hVWnd = m_pIWndPreview->GetWndHandle();

		m_pIWndPreview->SetRWndPreview(this);
		m_pIWndPreview->UpdateChildWndsInfo(hCWnd);
		m_pIWndPreview->ShowChildWndView();
	}
	else if (nID == ID_WINATT)
	{
		HWND hCWnd = mPtrWndEngine->GetWindow();
		HWND hAWnd = m_pIWndAttrib->GetWndHandle();
		m_pIWndAttrib->QueryWindowAttrib(hCWnd);
		SwitchToThisWindow(hAWnd, TRUE);
	}
	else if (nID == ID_ENUM_MODULE)
	{
		HWND hCWnd = mPtrWndEngine->GetWindow();
		HWND hMWnd = m_pIWndModule->GetWndHandle();
		m_pIWndModule->UpdateWndModuleInfo(mPtrWndEngine->GetStageInfo(IWndEngine::PRO_MODULES), hCWnd);
		SwitchToThisWindow(hMWnd, TRUE);
	}
	else if (nID == ID_QUIT)
	{
		FORWARD_WM_SYSCOMMAND(m_hWnd, SC_CLOSE, 0, 0, ::PostMessage);
	}
	else if (nID == ID_ABOUT)
	{
		CDialog	dlgAbout(IDD_ABOUTBOX, this);
		dlgAbout.DoModal();
	}
	else if (nID == ID_SET_COLOR)
	{
		CColorDialog cColorDialog(m_lSignColor, 0, this);
		if (cColorDialog.DoModal() == IDOK)
		{
			m_pIWndSigned->SetSignColor(m_lSignColor = cColorDialog.GetColor());
			if (m_pIFileEngine != NULL)
			{
				m_pIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_WIZARD_LONG_COLOR, &m_lSignColor);
			}
		}
	}
	else if (nID == ID_SET_FIND/* || (code == BN_CLICKED && nID == IDC_BUTTON_FINDMODE)*/)
	{
		m_FindOptionDlg.ShowWindow(SW_NORMAL);
	}
	else if (nID == ID_SET_FONT)
	{
		LOGFONT	LogFont = { -12,0,0,0,400,0,0,0,134,3,2,1,2,TEXT("宋体") };
		m_cFont.GetLogFont(&LogFont);
		GetFont()->GetLogFont(&LogFont);
		CFontDialog	cFontDialog(&LogFont, CF_EFFECTS | CF_SCREENFONTS, NULL, this);

		if (cFontDialog.DoModal() == IDOK)
		{
			m_cFont.DeleteObject();
			cFontDialog.GetCurrentFont(&LogFont);
#ifdef _DEBUG
			_tprintf(TEXT("设置字体：LOGFONT{%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\"%s\"}\n{\n"
				"\t%-8d:lfHeight\n"
				"\t%-8d:lfWidth\n"
				"\t%-8d:lfEscapement\n"
				"\t%-8d:lfOrientation\n"
				"\t%-8d:lfWeight\n"
				"\t%-8d:lfItalic\n"
				"\t%-8d:lfUnderline\n"
				"\t%-8d:lfStrikeOut\n"
				"\t%-8d:lfCharSet\n"
				"\t%-8d:lfOutPrecision\n"
				"\t%-8d:lfClipPrecision\n"
				"\t%-8d:lfQuality\n"
				"\t%-8d:lfPitchAndFamily\n"
				"\t%-8s:lfFaceName\n"
				"}\n"),
				LogFont.lfHeight, LogFont.lfWidth, LogFont.lfEscapement,
				LogFont.lfOrientation, LogFont.lfWeight, LogFont.lfItalic,
				LogFont.lfUnderline, LogFont.lfStrikeOut, LogFont.lfCharSet,
				LogFont.lfOutPrecision, LogFont.lfClipPrecision, LogFont.lfQuality,
				LogFont.lfPitchAndFamily, LogFont.lfFaceName,

				LogFont.lfHeight,
				LogFont.lfWidth,
				LogFont.lfEscapement,
				LogFont.lfOrientation,
				LogFont.lfWeight,
				LogFont.lfItalic,
				LogFont.lfUnderline,
				LogFont.lfStrikeOut,
				LogFont.lfCharSet,
				LogFont.lfOutPrecision,
				LogFont.lfClipPrecision,
				LogFont.lfQuality,
				LogFont.lfPitchAndFamily,
				LogFont.lfFaceName);
#endif
			m_cFont.CreateFontIndirect(&LogFont);
			EnumChildWindows(m_hWnd, SetChildFont, LPARAM(&m_cFont));

			if (m_pIFileEngine != NULL)
			{
				m_pIFileEngine->SetVarValue(IFileEngine::VT_STRUCT, FV_WIZARD_STRUCT_FONT, &LogFont);
			}
		}
	}
	else if (nID == IDC_BUTTON_OPENPROGRAM || nID == IDC_BUTTON_OPENMOUDLE)
	{
		TCHAR	szPath[MAX_PATH] = TEXT(""), command[MAX_PATH]= TEXT("");
		GetDlgItemText((nID == IDC_BUTTON_OPENPROGRAM) ? IDC_EDIT_PROCESSPATH : IDC_EDIT_MODULEPATH, szPath, MAX_PATH);
		if (szPath[0] != '\0')
		{
			StringCchPrintf(command, MAX_PATH, TEXT("/select,%s"), szPath);
			ShellExecute(NULL, TEXT("open"), TEXT("Explorer.exe"), command, TEXT(""), SW_SHOW);
		}
	}
	else if (nID == IDC_BUTTON_CHANGLETEXT || nID == IDC_BUTTON_SENDMESAGE)
	{
		TCHAR	szText[1024]= TEXT("");
		GetDlgItemText(IDC_EDIT_MESSAGE, szText, 1024);

		if (nID == IDC_BUTTON_CHANGLETEXT)
			this->mPtrWndEngine->SetWindowText(szText);
		else
			this->mPtrWndEngine->SendMessage(szText);
	}

	switch (nID)
	{
	case IDC_BUTTON_MAXSIZE:case IDC_BUTTON_MINSIZE:case IDC_BUTTON_NORMAL:
	case IDC_BUTTON_DISABLE:case IDC_BUTTON_ENABLE:case IDC_BUTTON_HIDING:
	case IDC_BUTTON_SHOWING:case IDC_BUTTON_TOPMOST:case IDC_BUTTON_DISTOPMOST:
	case IDC_BUTTON_CLOSE:case IDC_BUTTON_DESTROY:case IDC_BUTTON_KILLPROCESS:
	{
		//printf("nID = %-6d,code = %-6d,hCtl = %#x,idc = %d\n"),nID,code,hCtl,(nID == IDC_BUTTON_OPENPROGRAM)?IDC_EDIT_PROCESSPATH:IDC_EDIT_MODULEPATH);
		static	DWORD	ArrayId[] = {
			IDC_BUTTON_MAXSIZE,IDC_BUTTON_MINSIZE,IDC_BUTTON_NORMAL,
			IDC_BUTTON_DISABLE,IDC_BUTTON_ENABLE,IDC_BUTTON_HIDING,
			IDC_BUTTON_SHOWING,IDC_BUTTON_TOPMOST,IDC_BUTTON_DISTOPMOST,
			IDC_BUTTON_CLOSE,IDC_BUTTON_DESTROY,IDC_BUTTON_KILLPROCESS,
		};

		if (nID == IDC_BUTTON_KILLPROCESS && MessageBox(TEXT("确定要结束进程吗？"), TEXT("提问"), MB_ICONQUESTION | MB_TOPMOST | MB_YESNO) != IDYES)
		{
			return CWnd::OnCommand(wParam, lParam);
		}

		for (int i = 0, len = sizeof(ArrayId) / sizeof(ArrayId[0]); i < len; i++)
		{
			if (nID == ArrayId[i] && mPtrWndEngine->SendFunction(i) == false)
			{
				CString	szError;
				TCHAR	szText[32]= TEXT("");
				GetDlgItemText(nID, szText, 32);
				szError.Format(TEXT("%s失败:%s"), szText, mPtrWndEngine->GetStageInfo(IWndEngine::LAST_ERROR));

				MessageBox(szError, TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
				break;
			}
		}
	}
	}
	return CDialog::OnCommand(wParam, lParam);
}
// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWndWizardDlg::OnPaint()
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
HCURSOR CWndWizardDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWndWizardDlg::OnStnClickedFind()
{
	// TODO: 在此添加控件通知处理程序代码

	if (this->IsDlgButtonChecked(IDC_AUTOMODE) == FALSE)
		this->BeginFind();

}

void CWndWizardDlg::OnStnClickedMove()
{
	// TODO: 在此添加控件通知处理程序代码
	if (mPtrWndEngine->IsValidWindow()
		&& this->IsDlgButtonChecked(IDC_AUTOMODE) == FALSE)
	{
		if (::IsIconic(mPtrWndEngine->GetWindow()))
			MessageBox(TEXT("窗口处于最小化状态，不可移动。"), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
		else
			this->BeginMove();
	}

}
void CWndWizardDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialog::OnMouseMove(nFlags, point);

	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ClientToScreen(&point);

	if (m_Style == STYLE_MOVE)
	{
		static	HWND	hParent;
		static	RECT	rect;

		::GetWindowRect(mPtrWndEngine->GetWindow(), &rect);
		hParent = ::GetParent(mPtrWndEngine->GetWindow());

		if (hParent != NULL)::ScreenToClient(hParent, &point);

		::MoveWindow(mPtrWndEngine->GetWindow(), point.x, point.y,
			rect.right - rect.left, rect.bottom - rect.top, TRUE);
	}
	else if (m_Style == STYLE_FIND)
	{
		TOOLINFO    ti;//TOOLINFO是个存放控件提示信息的结构

		ti.cbSize = sizeof(TOOLINFO);//设定结构的大小
		ti.uFlags = TTF_IDISHWND;//指出uid成员是窗口的句柄
		ti.hwnd = m_hWnd;//包含提示的窗口的句柄
		ti.uId = 1;//应用程式定义的标识符


		if (mPtrWndEngine->FindWindow(&point, m_FindOptionDlg.m_uFlag) == true)
		{
			CString		szWndInfo, szFormat;

			LPRECT pRect = mPtrWndEngine->GetWndRect();
			szFormat.Format(TEXT("{%d,%d,%d,%d}"), pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);

			//szWndInfo.Format(TEXT("GetDpiForWindow:%u-%u-%u\r\n"), GetDpiForWindow(m_ToolTip.m_hWnd), GetDpiForWindow(mPtrWndEngine->GetWindow()),GetDpiForSystem());
			szWndInfo += "窗口标题:";
			szWndInfo += mPtrWndEngine->GetStageInfo(IWndEngine::WND_TEXT);
			szWndInfo += "\r\n";
			szWndInfo += "窗口类名:";
			szWndInfo += mPtrWndEngine->GetStageInfo(IWndEngine::WND_CLSNAME);
			szWndInfo += "\r\n";
			szWndInfo += "进程名称:";
			szWndInfo += mPtrWndEngine->GetStageInfo(IWndEngine::WND_PROCESSNAME);
			szWndInfo += "\r\n";
			szWndInfo += "模块名称:";
			szWndInfo += mPtrWndEngine->GetStageInfo(IWndEngine::WND_MODULENAME);
			szWndInfo += "\r\n";
			szWndInfo += "窗口句柄:";
			szWndInfo += mPtrWndEngine->GetStageInfo(IWndEngine::WND_HANDLE);
			szWndInfo += "\r\n";
			szWndInfo += "窗口矩形:";
			szWndInfo += szFormat;
			//szWndInfo += "\r\n";
			m_ToolTip.UpdateTipText(szWndInfo, this, 1);

			CRect	cRtTooltip, cRtDesktop;
			m_ToolTip.GetWindowRect(&cRtTooltip);
			GetDesktopWindow()->GetWindowRect(&cRtDesktop);

			CPoint mousePoint;

			mousePoint.x = point.x + 25;
			mousePoint.y = point.y + 25;

			if (mousePoint.x + cRtTooltip.Width() > cRtDesktop.Width()) {
				mousePoint.x -= 50;
				mousePoint.x -= cRtTooltip.Width();
			}
			if (mousePoint.y + cRtTooltip.Height() > cRtDesktop.Height()) {
				mousePoint.y -= 50;
				mousePoint.y -= cRtTooltip.Height();
			}
			m_ToolTip.MoveWindow(mousePoint.x, mousePoint.y, cRtTooltip.Width(), cRtTooltip.Height(), TRUE);
			m_ToolTip.SendMessage(TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);

			HWND hCWnd = mPtrWndEngine->GetWindow();
			HWND hMWnd = m_pIWndModule->GetWndHandle();
			HWND hVWnd = m_pIWndPreview->GetWndHandle();
			HWND hAWnd = m_pIWndAttrib->GetWndHandle();

			this->UpDateUserInterface();

			m_pIWndSigned->SignRect(mPtrWndEngine->GetWndRect());

			if (::IsWindowVisible(hMWnd) && ::IsIconic(hMWnd) == FALSE)
			{
				DWORD dwPid, dwTid = GetWindowThreadProcessId(hCWnd, &dwPid);

				if (m_dwPreModulePid != dwPid)
				{
					m_dwPreModulePid = dwPid;
					m_pIWndModule->UpdateWndModuleInfo(mPtrWndEngine->GetStageInfo(IWndEngine::PRO_MODULES), hCWnd);
				}
			}
			if (::IsWindowVisible(hVWnd) && ::IsIconic(hVWnd) == FALSE)
			{
				DWORD dwPid, dwTid = GetWindowThreadProcessId(hCWnd, &dwPid);

				if (m_hPreViewHwnd != hCWnd)
				{
					m_hPreViewHwnd = hCWnd;
					m_pIWndPreview->UpdateChildWndsInfo(m_hPreViewHwnd);
				}
			}
			if (::IsWindowVisible(hAWnd) && ::IsIconic(hAWnd) == FALSE)
			{
				m_pIWndAttrib->QueryWindowAttrib(hCWnd);
			}
		}

		if (m_ToolTip.IsWindowVisible() && mPtrWndEngine->GetWindow(IWndEngine::GETWINDOW_CURWND)) {

			CRect	cRtTooltip, cRtDesktop;
			m_ToolTip.GetWindowRect(&cRtTooltip);
			GetDesktopWindow()->GetWindowRect(&cRtDesktop);

			CPoint mousePoint;

			mousePoint.x = point.x + 25;
			mousePoint.y = point.y + 25;

			if (mousePoint.x + cRtTooltip.Width() > cRtDesktop.Width()) {
				mousePoint.x -= 50;
				mousePoint.x -= cRtTooltip.Width();
			}
			if (mousePoint.y + cRtTooltip.Height() > cRtDesktop.Height()) {
				mousePoint.y -= 50;
				mousePoint.y -= cRtTooltip.Height();
			}
			m_ToolTip.MoveWindow(mousePoint.x, mousePoint.y, cRtTooltip.Width(), cRtTooltip.Height(), TRUE);
			//m_ToolTip.SendMessage(TTM_TRACKACTIVATE,(WPARAM)TRUE,(LPARAM)&ti);
		}
	}

}
void CWndWizardDlg::ReleaseMouseCapture()
{
	if (m_Style == STYLE_MOVE)
		this->EndMove();
	else if (m_Style == STYLE_FIND)
		this->EndFind();
	if (m_ToolTip.IsWindowVisible()) {
		m_ToolTip.ShowWindow(SW_HIDE);
	}
}
void CWndWizardDlg::UpDateUserInterface()
{
	bool isValidWindow = false;

	if (isValidWindow = mPtrWndEngine->IsValidWindow())
	{
		static	DWORD	ArrayId[] = {
			IDC_EDIT_WIN_HANDLE,IDC_EDIT_WIN_CAPTURE,
			IDC_EDIT_WIN_CLASSNAME,IDC_EDIT_WIN_CLASSVALUE,
			IDC_EDIT_WIN_STYLE,IDC_EDIT_WIN_EXSTYLE,
			IDC_EDIT_WIN_PHANDLE,IDC_EDIT_WIN_PCAPTURE,
			IDC_EDIT_WIN_PCLASSNAME,IDC_EDIT_PROCESSID,IDC_EDIT_THREADID,
			IDC_EDIT_MODULENAME,IDC_EDIT_MODULEPATH,
			IDC_EDIT_PROCESSNAME,IDC_EDIT_PROCESSPATH,
			IDC_EDIT_PROCESSHINST,IDC_EDIT_MODULEHINST
		};
		static	DWORD	FunctionId[] = {
			IWndEngine::WND_HANDLE		,IWndEngine::WND_TEXT,
			IWndEngine::WND_CLSNAME		,IWndEngine::WND_CLSVALUE,
			IWndEngine::WND_STYLE		,IWndEngine::WND_EXSTYLE,
			IWndEngine::WND_PHANDLE		,IWndEngine::WND_PTEXT,
			IWndEngine::WND_PCLSNAME	,IWndEngine::WND_PROCESSID	,IWndEngine::PROC_THREADID,
			IWndEngine::WND_MODULENAME	,IWndEngine::WND_MODULEPATH,
			IWndEngine::WND_PROCESSNAME	,IWndEngine::WND_PROCESSPATH,
			IWndEngine::WND_PEOCESSHINST,IWndEngine::WND_MODULEHINST
		};

		for (int i = 0, len = sizeof(ArrayId) / sizeof(ArrayId[0]); i < len; i++)
		{
			SetDlgItemText(ArrayId[i], mPtrWndEngine->GetStageInfo(/*IWndEngine::WND_PROCESSPATH/**/FunctionId[i]));
		}

		TCHAR	szTemp[32];
		LPRECT	lpRect = mPtrWndEngine->GetWndRect();
		StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%d"), lpRect->left);
		SetDlgItemText(IDC_EDIT_LEFT, szTemp);
		StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%d"), lpRect->top);
		SetDlgItemText(IDC_EDIT_TOP, szTemp);
		StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%d"), lpRect->right);
		SetDlgItemText(IDC_EDIT_RIGHT, szTemp);
		StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%d"), lpRect->bottom);
		SetDlgItemText(IDC_EDIT_BOTTOM, szTemp);
		StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%d"), lpRect->right - lpRect->left);
		SetDlgItemText(IDC_EDIT_WIDTH, szTemp);
		StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%d"), lpRect->bottom - lpRect->top);
		SetDlgItemText(IDC_EDIT_HEIGHT, szTemp);


		//if(m_WinAttDlg.IsWindowVisible())
		//		m_WinAttDlg.SetCheckWindow(mPtrWndEngine->GetWindow());

	}

	static	DWORD	ArrayItemId[] = {
		IDC_BUTTON_MAXSIZE,IDC_BUTTON_MINSIZE,IDC_BUTTON_NORMAL,
		IDC_BUTTON_DISABLE,IDC_BUTTON_ENABLE,IDC_BUTTON_HIDING,
		IDC_BUTTON_SHOWING,IDC_BUTTON_TOPMOST,IDC_BUTTON_DISTOPMOST,
		IDC_BUTTON_CLOSE,IDC_BUTTON_DESTROY,IDC_BUTTON_KILLPROCESS,
		IDC_BUTTON_PICKUP,IDC_BUTTON_CHANGLE,IDC_BUTTON_CHANGLETEXT,
		IDC_BUTTON_SENDMESAGE,IDC_BUTTON_TOPARENT,
	};

	for (int i = 0, len = sizeof(ArrayItemId) / sizeof(ArrayItemId[0]); i < len; i++)
	{
		::EnableWindow(::GetDlgItem(this->m_hWnd, ArrayItemId[i]), isValidWindow);
	}

}
void CWndWizardDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonUp(nFlags, point);
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (this->IsDlgButtonChecked(IDC_AUTOMODE) == FALSE)
		ReleaseMouseCapture();
}

void CWndWizardDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (this->m_bIsBackstage == false) {
		CDialog::OnClose();

		m_pIWndPreview->DelRWndPreview(this);
		m_pIViewWizard->ClearViewWizard(this);

		if (g_WndWizardDlgNumber == 0)
		{
			//CDialog::OnCommand(MAKEWPARAM(ID_APP_EXIT,0),NULL);
		}
	}
	else {
		FORWARD_WM_SYSCOMMAND(m_hWnd, SC_MINIMIZE, 0, 0, ::PostMessage);
	}
}

void CWndWizardDlg::OnBnClickedButtonOkCancel()
{
}
void CWndWizardDlg::OnBnClickedTopmost()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bIsTopMost = IsDlgButtonChecked(IDC_TOPMOST) != 0;

	if (m_bIsTopMost == true)
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	else
		::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);


	if (m_pIFileEngine != NULL)
	{
		m_pIFileEngine->SetVarValue(IFileEngine::VT_BOOL, FV_WIZARD_BOOL_TOPMOST, &m_bIsTopMost);
	}
}

void CWndWizardDlg::OnBnClickedGray()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bIsEnGary = IsDlgButtonChecked(IDC_GRAY) != 0;
	m_FindOptionDlg.SetFindOption(2 * m_bIsEnGary - 1);
	if (m_pIFileEngine != NULL)
	{
		m_pIFileEngine->SetVarValue(IFileEngine::VT_BOOL, FV_WIZARD_BOOL_ENGARY, &m_bIsEnGary);
	}
}
void CWndWizardDlg::OnBnClickedButtonPickup()
{
	// TODO: 在此添加控件通知处理程序代码
	m_pIWndExtract->AddBorderWnd(this->mPtrWndEngine->GetWindow());
}

void CWndWizardDlg::OnBnClickedButtonFindwindow()
{
	// TODO: 在此添加控件通知处理程序代码
	TCHAR	szClass[128]= TEXT("");
	TCHAR	szTitle[128]= TEXT("");
	this->GetDlgItemText(IDC_EDIT_WIN_CLASSNAME, szClass, 128);
	this->GetDlgItemText(IDC_EDIT_WIN_CAPTURE, szTitle, 128);

	::EnableWindow(::GetDlgItem(this->m_hWnd, IDC_AUTOMODE), FALSE);
	::EnableWindow(::GetDlgItem(this->m_hWnd, IDC_FIND), FALSE);
	m_FindWindowDlg.SetFindParam(szClass, szTitle, mPtrWndEngine->GetWindow());
	m_FindWindowDlg.ShowWindow(SW_NORMAL);
}
LRESULT CWndWizardDlg::OnHotKey(WPARAM wParam, LPARAM lParam)
{
	if (wParam == HT_ENDAUTOMODE)
	{
		if (m_bIsBackstage == false) {
			this->EndFind();
			this->KillTimer(IDT_FIND);
			::EnableWindow(::GetDlgItem(this->m_hWnd, IDC_BUTTON_FINDWINDOW), TRUE);
			::UnregisterHotKey(m_hWnd, HT_ENDAUTOMODE);
			this->CheckDlgButton(IDC_AUTOMODE, FALSE);

			if (m_ToolTip.IsWindowVisible()) {
				m_ToolTip.ShowWindow(SW_HIDE);
			}
		}
	}
	else if (HT_OPENMAINPANEL == wParam) {
		if (this->IsWindowVisible() == FALSE) {
			this->CenterWindow();
			this->ShowWindow(SW_SHOWNORMAL);
		}
	}
	else if (HT_AUTOMODEBEGIN == wParam) {
		if (m_Style == STYLE_NULL) {
			this->BeginFind();
			::ReleaseCapture();
			this->SetTimer(IDT_FIND, 5, NULL);
			::EnableWindow(::GetDlgItem(this->m_hWnd, IDC_BUTTON_FINDWINDOW), FALSE);
			this->CheckDlgButton(IDC_AUTOMODE, TRUE);
		}
	}
	else if (HT_AUTOMODEEND == wParam) {

		if (m_Style == STYLE_FIND) {
			this->EndFind();
			this->KillTimer(IDT_FIND);
			::EnableWindow(::GetDlgItem(this->m_hWnd, IDC_BUTTON_FINDWINDOW), TRUE);
			this->CheckDlgButton(IDC_AUTOMODE, FALSE);

			if (m_ToolTip.IsWindowVisible()) {
				m_ToolTip.ShowWindow(SW_HIDE);
			}
		}
	}
	else if (HT_FINDCPUPAGE == wParam)
	{
//		if (m_pIProcEngine->UpdateProcessInfo())
//		{
//			LPPROCCONFIG lpConfig = 0;
//			PROCCONFIG 	sysProcConfig;
//			CString		cReport= TEXT("");
//			CArray<PROCCONFIG>	ConfigArry;
//			ULONG		uAllCPUPage = 0;
//
//			while (lpConfig = m_pIProcEngine->GetProcessConfig())
//			{
//				PROCCONFIG ProcConfig = *lpConfig;
//				uAllCPUPage += ProcConfig.ulCPUPage;
//				ConfigArry.Add(ProcConfig);
//			}
//
//			if (uAllCPUPage < 90)
//			{
//				cReport.Format(TEXT("获取进程信息错误 uAllCPUPage = %d\r\n\r\n"), uAllCPUPage);
//				for (INT_PTR i = 0, len = ConfigArry.GetSize(); i < len; i++)
//				{
//					CString	szFormat;
//					PROCCONFIG &iProcConfig = ConfigArry.ElementAt(i);
//					szFormat.Format(TEXT("%06d  %02d  %s\n"), iProcConfig.dwProcessID, iProcConfig.ulCPUPage, iProcConfig.szProcessName);
//					cReport += szFormat;
//				}
//				MessageBox(cReport, TEXT("提示"), MB_ICONWARNING | MB_TOPMOST);
//				return TRUE;
//			}
//
//			for (INT_PTR i = 0, len = ConfigArry.GetSize(); i < len; i++)
//			{
//				PROCCONFIG &iProcConfig = ConfigArry.ElementAt(i);
//#ifdef DEBUG
//				_tprintf(TEXT("%06d\t%s\n"), iProcConfig.dwProcessID, iProcConfig.szProcessName);
//#endif // DEBUG
//
//				if (iProcConfig.dwProcessID == 0)
//				{
//					sysProcConfig = iProcConfig;
//				}
//				for (INT_PTR j = i; j < len; j++)
//				{
//					PROCCONFIG &jProcConfig = ConfigArry.ElementAt(j);
//
//					if (iProcConfig.ulCPUPage < jProcConfig.ulCPUPage) {
//						PROCCONFIG tProcConfig = iProcConfig;
//						iProcConfig = jProcConfig;
//						jProcConfig = tProcConfig;
//					}
//				}
//			}
//
//			if (sysProcConfig.ulCPUPage > 2)
//			{
//				cReport.Format(TEXT("找不到CPU占用高的进程 uAllCPUPage = %d\r\n\r\n"), uAllCPUPage);
//				for (INT_PTR i = 0, len = ConfigArry.GetSize(); i < len; i++)
//				{
//					CString	szFormat;
//					PROCCONFIG &iProcConfig = ConfigArry.ElementAt(i);
//					szFormat.Format(TEXT("%06d  %02d  %s\n"), iProcConfig.dwProcessID, iProcConfig.ulCPUPage, iProcConfig.szProcessName);
//					cReport += szFormat;
//				}
//				cReport += "\r\n";
//				//MessageBox(cReport,"提示"),MB_ICONWARNING|MB_TOPMOST);
//				//return TRUE;
//				//cReport += "找不到CPU占用高的进程！\r\n";
//			}
//			else
//			{
//				ULONG	AllCPUPage = 0;
//
//				for (INT_PTR i = 0, len = ConfigArry.GetSize(); i < len; i++)
//				{
//					PROCCONFIG &iProcConfig = ConfigArry.ElementAt(i);
//
//					AllCPUPage += iProcConfig.ulCPUPage;
//
//					CString	szFormat;
//
//					szFormat.Format(TEXT("%02d"), iProcConfig.ulCPUPage);
//
//					cReport += "CPU[";
//					cReport += szFormat;
//					cReport += "%] ";
//					cReport += iProcConfig.szProcessName;
//					cReport += "\r\n设置进程";
//					cReport += iProcConfig.szProcessName;
//					cReport += "最低优先级";
//
//					if (mPtrWndEngine->SetPriorityClass(iProcConfig.dwProcessID, IWndEngine::PRIORITY_CLASS_IDLE))
//					{
//						cReport += "成功\r\n\r\n";
//					}
//					else
//					{
//						cReport += "失败[";
//						cReport += mPtrWndEngine->GetStageInfo(IWndEngine::LAST_ERROR);
//						if (cReport[cReport.GetLength() - 1] == '\n')
//						{
//							cReport = cReport.Left(cReport.GetLength() - 1);
//						}
//						cReport += "]\r\n结束进程";
//						cReport += iProcConfig.szProcessName;
//
//						if (mPtrWndEngine->QueryProcess(iProcConfig.dwProcessID) == false)
//						{
//							cReport += "失败[";
//							cReport += mPtrWndEngine->GetStageInfo(IWndEngine::LAST_ERROR);
//							if (cReport[cReport.GetLength() - 1] == '\n')
//							{
//								cReport = cReport.Left(cReport.GetLength() - 1);
//							}
//							cReport += "]\r\n结束进程";
//						}
//						if (mPtrWndEngine->SendFunction(IWndEngine::FUN_KILLPROCESS) == true)
//						{
//							cReport += "成功\r\n\r\n";
//						}
//						else
//						{
//							cReport += "失败[";
//							cReport += mPtrWndEngine->GetStageInfo(IWndEngine::LAST_ERROR);
//							if (cReport[cReport.GetLength() - 1] == '\n')
//							{
//								cReport = cReport.Left(cReport.GetLength() - 1);
//							}
//							cReport += "]\r\n\r\n";
//						}
//					}
//					if (AllCPUPage > 50)
//					{
//						break;
//					}
//				}
//			}
//			MessageBox(cReport, TEXT("提示"), MB_ICONWARNING | MB_TOPMOST);
//		}
	}
	return TRUE;
}
void CWndWizardDlg::OnBnClickedAutoMode()
{
	// TODO: 在此添加控件通知处理程序代码
	if (this->IsDlgButtonChecked(IDC_AUTOMODE))
	{
		this->BeginFind();
		::ReleaseCapture();
		this->SetTimer(IDT_FIND, 5, NULL);
		::EnableWindow(::GetDlgItem(this->m_hWnd, IDC_BUTTON_FINDWINDOW), FALSE);

		if (this->m_bIsBackstage == false) {
			static bool bIsShow = false;
			BOOL ret = RegisterHotKey(m_hWnd, HT_ENDAUTOMODE, m_wEndFind1, m_wEndFind2);
			if (ret != FALSE && bIsShow == false)
			{
				bIsShow = true;
				HWND hFrameWnd = AfxGetApp()->m_pMainWnd->m_hWnd;
				mCtlHotKey.SetHotKey(m_wEndFind2, m_wEndFind1);
				::SendMessage(hFrameWnd, FM_NOTIFYMSG, WPARAM(TEXT("提示")), LPARAM(LPCTSTR(CString("退出自动模式可以按快捷键") + mCtlHotKey.GetHotKeyName())));
			}
		}
	}
	else
	{
		this->EndFind();
		this->KillTimer(IDT_FIND);
		::EnableWindow(::GetDlgItem(this->m_hWnd, IDC_BUTTON_FINDWINDOW), TRUE);
		::UnregisterHotKey(m_hWnd, HT_ENDAUTOMODE);

		if (m_ToolTip.IsWindowVisible()) {
			m_ToolTip.ShowWindow(SW_HIDE);
		}
	}
}
void CWndWizardDlg::OnBnClickedHidemode()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pIFileEngine != NULL)
	{
		m_bIsHideMode = IsDlgButtonChecked(IDC_HIDEMODE) != 0;
		m_pIFileEngine->SetVarValue(IFileEngine::VT_BOOL, FV_WIZARD_BOOL_HIDEMODE, &m_bIsHideMode);
	}

}
void CWndWizardDlg::OnBnClickedButtonChangle()
{
	// TODO: 在此添加控件通知处理程序代码
	RECT	rect = { 0,0,0,0 };
	TCHAR	szTemp[32]= TEXT("");

	this->GetDlgItemText(IDC_EDIT_LEFT, szTemp, 32);
	rect.left = _tcstol(szTemp, NULL, 0);
	this->GetDlgItemText(IDC_EDIT_TOP, szTemp, 32);
	rect.top = _tcstol(szTemp, NULL, 0);
	this->GetDlgItemText(IDC_EDIT_RIGHT, szTemp, 32);
	rect.right = _tcstol(szTemp, NULL, 0);
	this->GetDlgItemText(IDC_EDIT_BOTTOM, szTemp, 32);
	rect.bottom = _tcstol(szTemp, NULL, 0);

	if (false == mPtrWndEngine->SetWinRect(&rect))
	{
		MessageBox(CString("设置窗口位置出错：") + mPtrWndEngine->GetStageInfo(IWndEngine::LAST_ERROR), TEXT("提示"), MB_TOPMOST | MB_ICONERROR);
	}

}

void CWndWizardDlg::OnBnClickedButtonUpdate()
{
	// TODO: 在此添加控件通知处理程序代码
	mPtrWndEngine->UpDateWndInfo();
	this->UpDateUserInterface();
}
void CWndWizardDlg::OnBnClickedButtonToparent()
{
	// TODO: 在此添加控件通知处理程序代码	
	if (mPtrWndEngine->TurnToParent() == true)
		this->UpDateUserInterface();
	else
		MessageBox(CString("转至父窗口失败：") + mPtrWndEngine->GetStageInfo(IWndEngine::LAST_ERROR), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
}

void CWndWizardDlg::OnKillFocus(CWnd* pNewWnd)
{
	CDialog::OnKillFocus(pNewWnd);

	// TODO: 在此处添加消息处理程序代码
	if (this->IsDlgButtonChecked(IDC_AUTOMODE) == FALSE)
		this->ReleaseMouseCapture();
}

void CWndWizardDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);

	// TODO: 在此处添加消息处理程序代码
	if (this->IsDlgButtonChecked(IDC_AUTOMODE) == FALSE)
		this->ReleaseMouseCapture();
}

void CWndWizardDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialog::OnTimer(nIDEvent);
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == IDT_FIND && this->m_Style == this->STYLE_FIND)
	{
		TOOLINFO    ti;//TOOLINFO是个存放控件提示信息的结构

		ti.cbSize = sizeof(TOOLINFO);//设定结构的大小
		ti.uFlags = TTF_IDISHWND;//指出uid成员是窗口的句柄
		ti.hwnd = m_hWnd;//包含提示的窗口的句柄
		ti.uId = 1;//应用程式定义的标识符

		POINT ptMouse;
		::GetCursorPos(&ptMouse);

		if (mPtrWndEngine->FindWindow(&ptMouse, m_FindOptionDlg.m_uFlag) == true)
		{

			CString szWndInfo, szFormat;

			LPRECT pRect = mPtrWndEngine->GetWndRect();
			szFormat.Format(TEXT("{%d,%d,%d,%d}"), pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);

			szWndInfo += "窗口标题:";
			szWndInfo += mPtrWndEngine->GetStageInfo(IWndEngine::WND_TEXT);
			szWndInfo += "\r\n";
			szWndInfo += "窗口类名:";
			szWndInfo += mPtrWndEngine->GetStageInfo(IWndEngine::WND_CLSNAME);
			szWndInfo += "\r\n";
			szWndInfo += "进程名称:";
			szWndInfo += mPtrWndEngine->GetStageInfo(IWndEngine::WND_PROCESSNAME);
			szWndInfo += "\r\n";
			szWndInfo += "模块名称:";
			szWndInfo += mPtrWndEngine->GetStageInfo(IWndEngine::WND_MODULENAME);
			szWndInfo += "\r\n";
			szWndInfo += "窗口句柄:";
			szWndInfo += mPtrWndEngine->GetStageInfo(IWndEngine::WND_HANDLE);
			szWndInfo += "\r\n";
			szWndInfo += "窗口矩形:";
			szWndInfo += szFormat;
			//szWndInfo += "\r\n";
			m_ToolTip.UpdateTipText(szWndInfo, this, 1);


			CPoint mousePoint;
			CRect	cRtTooltip, cRtDesktop;
			m_ToolTip.GetWindowRect(&cRtTooltip);
			GetDesktopWindow()->GetWindowRect(&cRtDesktop);

			mousePoint.x = ptMouse.x + 25;
			mousePoint.y = ptMouse.y + 25;

			if (mousePoint.x + cRtTooltip.Width() > cRtDesktop.Width()) {
				mousePoint.x -= 50;
				mousePoint.x -= cRtTooltip.Width();
			}
			if (mousePoint.y + cRtTooltip.Height() > cRtDesktop.Height()) {
				mousePoint.y -= 50;
				mousePoint.y -= cRtTooltip.Height();
			}

			m_ToolTip.MoveWindow(mousePoint.x, mousePoint.y, cRtTooltip.Width(), cRtTooltip.Height(), TRUE);
			m_ToolTip.SendMessage(TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);

			this->UpDateUserInterface();
			m_pIWndSigned->SignRect(mPtrWndEngine->GetWndRect());


		}
		if (m_ToolTip.IsWindowVisible() && mPtrWndEngine->GetWindow(IWndEngine::GETWINDOW_CURWND)) {

			CPoint mousePoint;
			CRect	cRtTooltip, cRtDesktop;
			m_ToolTip.GetWindowRect(&cRtTooltip);
			GetDesktopWindow()->GetWindowRect(&cRtDesktop);

			mousePoint.x = ptMouse.x + 25;
			mousePoint.y = ptMouse.y + 25;

			if (mousePoint.x + cRtTooltip.Width() > cRtDesktop.Width()) {
				mousePoint.x -= 50;
				mousePoint.x -= cRtTooltip.Width();
			}
			if (mousePoint.y + cRtTooltip.Height() > cRtDesktop.Height()) {
				mousePoint.y -= 50;
				mousePoint.y -= cRtTooltip.Height();
			}
			m_ToolTip.MoveWindow(mousePoint.x, mousePoint.y, cRtTooltip.Width(), cRtTooltip.Height(), TRUE);
			//m_ToolTip.SendMessage(TTM_TRACKACTIVATE,(WPARAM)TRUE,(LPARAM)&ti);
		}
	}
	else if (nIDEvent == IDT_CHECK) {
		// TODO: 在此添加控件通知处理程序代码
		if (m_pIProcEngine == nullptr || m_pIFileEngine == nullptr || m_pIWndModule == nullptr || m_pIWndPreview == nullptr) {
			this->KillTimer(IDT_CHECK);
			CWnd::MessageBox(TEXT("句柄已经丢失！"));
		}
	}

}
void CWndWizardDlg::BeginFind()
{
	this->GetWindowRect(&m_wRect);

	if (IsDlgButtonChecked(IDC_HIDEMODE) && IsDlgButtonChecked(IDC_AUTOMODE) == FALSE)
		::SetWindowPos(m_hWnd, HWND_TOP, 0, 2000, 0, 0, TRUE);

	if (mPtrWndEngine->IsValidWindow()
		&& m_pIWndSigned->IsSignHide() == true)
	{
		mPtrWndEngine->UpDateWndInfo();
		m_pIWndSigned->SignRect(mPtrWndEngine->GetWndRect());
		this->UpDateUserInterface();
	}

	mCurFind.SetCursor(mHcurEmpty);
	::SetCursor(mHcurFinder);
	this->SetCapture();
	m_Style = STYLE_FIND;
}
void CWndWizardDlg::BeginMove()
{
	RECT	rect;
	::GetWindowRect(mPtrWndEngine->GetWindow(), &rect);
	::SetCursorPos(rect.left, rect.top);

	mCurMove.SetCursor(mHcurEmpty);
	::SetCursor(mHcurHander);
	this->SetCapture();
	m_Style = STYLE_MOVE;
}
void CWndWizardDlg::EndFind()
{
	mCurFind.SetCursor(mHcurFinder);

	::ReleaseCapture();
	this->m_pIWndSigned->SignHide();

	if (this->IsDlgButtonChecked(IDC_HIDEMODE))
		::SetWindowPos(m_hWnd, HWND_TOP, m_wRect.left, m_wRect.top, 0, 0, TRUE);

	m_Style = STYLE_NULL;
}
void CWndWizardDlg::EndMove()
{
	mCurMove.SetCursor(mHcurHander);
	::ReleaseCapture();

	RECT	rect;
	::GetWindowRect(GetDlgItem(IDC_MOVE)->m_hWnd, &rect);
	::SetCursorPos((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);

	m_Style = STYLE_NULL;
}

void CWndWizardDlg::OnFindWindow(HWND hwnd)
{
	if (mPtrWndEngine->QueryWindow(hwnd))
	{
		m_pIWndSigned->SignRect(mPtrWndEngine->GetWndRect());
		this->UpDateUserInterface();
	}
}
bool CWndWizardDlg::OnCheckRFindWindow()
{
	return true;
}
void CWndWizardDlg::OnEndFindWindow()
{
	if (m_pIWndSigned->IsSignHide() == false)
		m_pIWndSigned->SignHide();
	::EnableWindow(::GetDlgItem(this->m_hWnd, IDC_AUTOMODE), TRUE);
	::EnableWindow(::GetDlgItem(this->m_hWnd, IDC_FIND), TRUE);
}
bool CWndWizardDlg::OnQueryProcess(DWORD dwPid)
{
	if (mPtrWndEngine->QueryProcess(dwPid))
	{
		HWND	hCWnd = mPtrWndEngine->GetWindow();
		LPCTSTR	lpStr = mPtrWndEngine->GetStageInfo(IWndEngine::PRO_MODULES);
		return this->m_pIWndModule->UpdateWndModuleInfo(lpStr, hCWnd, dwPid);
	}
	return false;
}
bool CWndWizardDlg::OnQueryWindow(HWND hwnd)
{
	if (mPtrWndEngine->QueryWindow(hwnd))
	{
		CRect	cRect;
		this->CenterWindow();
		this->UpDateUserInterface();
		this->GetWindowRect(&cRect);
		m_pIWndPreview->HideChildWndView();
		m_pIWndSigned->SignRect(&cRect, 1000);
		return true;
	}
	return false;
}
LPCTSTR	CWndWizardDlg::OnGetLastError()
{
	return mPtrWndEngine->GetStageInfo(IWndEngine::LAST_ERROR);
}
bool CWndWizardDlg::OnCheckRWndPreview()
{
	return true;
}

BOOL CWndWizardDlg::OnQueryOpen()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	HWND hTrayWnd = ::FindWindow(TEXT("Shell_TrayWnd"), NULL);  //windows底部信息提示窗口
	HWND hNotifyWnd = ::FindWindowEx(hTrayWnd, NULL, TEXT("TrayNotifyWnd"), NULL);
	if (hTrayWnd && hNotifyWnd)
	{
		CRect rtNotify;
		::GetWindowRect(hNotifyWnd, &rtNotify);
		rtNotify.right -= 25;
		rtNotify.bottom -= 5;
		::DrawAnimatedRects(m_hWnd, IDANI_CAPTION, &rtNotify, &m_wRect);
	}
	return __super::OnQueryOpen();
}

bool CWndWizardDlg::SetIWndModule(IWndModule*pIWndModule)
{
	m_pIWndModule = pIWndModule;
	m_pIProcEngine = (IProcEngine*)(pIWndModule->GetProcEngine());
	return pIWndModule != NULL;
}
bool CWndWizardDlg::SetIWndPreview(IWndPreview*pIWndPreview)
{
	m_pIWndPreview = pIWndPreview;
	return pIWndPreview != NULL;
}
bool CWndWizardDlg::SetIFileEngine(IFileEngine*pIFileEngine)
{
	m_pIFileEngine = pIFileEngine;
	return m_pIFileEngine != NULL;
}
BOOL CALLBACK CWndWizardDlg::SetChildFont(HWND hWnd, LPARAM lparam)
{
	::SendMessage(hWnd, WM_SETFONT, WPARAM(((CFont*)lparam)->GetSafeHandle()), 1);
	//EnumChildWindows(hWnd,SetChildFont,lparam);
	return TRUE;
}



//void CWndWizardDlg::OnBnClickedButtonFindmode()
//{
//	// TODO: 在此添加控件通知处理程序代码
//}


void CWndWizardDlg::OnSetHotkey()
{
	// TODO: 在此添加命令处理程序代码
	CWndWizHotkeyDlg cHotkeyDlg(this);

	cHotkeyDlg.m_wMainPanel1 = m_wMainPanel1;
	cHotkeyDlg.m_wMainPanel2 = m_wMainPanel2;
	cHotkeyDlg.m_wBackBegin1 = m_wBackBegin1;
	cHotkeyDlg.m_wBackBegin2 = m_wBackBegin2;
	cHotkeyDlg.m_wBackEnd1 = m_wBackEnd1;
	cHotkeyDlg.m_wBackEnd2 = m_wBackEnd2;
	cHotkeyDlg.m_wEndFind1 = m_wEndFind1;
	cHotkeyDlg.m_wEndFind2 = m_wEndFind2;
	cHotkeyDlg.m_wFindCpu1 = m_wFindCpu1;
	cHotkeyDlg.m_wFindCpu2 = m_wFindCpu2;

	if (cHotkeyDlg.DoModal() == IDOK)
	{
		if (m_bIsBackstage == TRUE) {
			if (m_wMainPanel1 != cHotkeyDlg.m_wMainPanel1 || m_wBackBegin2 != cHotkeyDlg.m_wMainPanel2)
			{
				::UnregisterHotKey(m_hWnd, HT_OPENMAINPANEL);
				if (RegisterHotKey(m_hWnd, HT_OPENMAINPANEL, cHotkeyDlg.m_wMainPanel1, cHotkeyDlg.m_wMainPanel2) == FALSE)
				{
					mCtlHotKey.SetHotKey(cHotkeyDlg.m_wMainPanel2, cHotkeyDlg.m_wMainPanel1);
					AfxMessageBox(CString(TEXT("注册快捷键【")) + mCtlHotKey.GetHotKeyName() + TEXT("】失败！原因可能是快捷键与别的程序冲突，请更新设置快捷键"));
				}
			}
			if (m_wBackBegin1 != cHotkeyDlg.m_wBackBegin1 || m_wBackBegin2 != cHotkeyDlg.m_wBackBegin2)
			{
				::UnregisterHotKey(m_hWnd, HT_AUTOMODEBEGIN);
				if (RegisterHotKey(m_hWnd, HT_AUTOMODEBEGIN, cHotkeyDlg.m_wBackBegin1, cHotkeyDlg.m_wBackBegin2) == FALSE)
				{
					mCtlHotKey.SetHotKey(cHotkeyDlg.m_wBackBegin2, cHotkeyDlg.m_wBackBegin1);
					AfxMessageBox(CString(TEXT("注册快捷键【")) + mCtlHotKey.GetHotKeyName() + TEXT("】失败！原因可能是快捷键与别的程序冲突，请更新设置快捷键"));
				}
			}
			if (m_wBackEnd1 != cHotkeyDlg.m_wBackEnd1 || m_wBackEnd2 != cHotkeyDlg.m_wBackEnd2)
			{
				::UnregisterHotKey(m_hWnd, HT_AUTOMODEEND);
				if (RegisterHotKey(m_hWnd, HT_AUTOMODEEND, cHotkeyDlg.m_wBackEnd1, cHotkeyDlg.m_wBackEnd2) == FALSE)
				{
					mCtlHotKey.SetHotKey(cHotkeyDlg.m_wBackEnd2, cHotkeyDlg.m_wBackEnd1);
					AfxMessageBox(CString(TEXT("注册快捷键【")) + mCtlHotKey.GetHotKeyName() + TEXT("】失败！原因可能是快捷键与别的程序冲突，请更新设置快捷键"));
				}
			}
			if (m_wFindCpu1 != cHotkeyDlg.m_wFindCpu1 || m_wFindCpu2 != cHotkeyDlg.m_wFindCpu2)
			{
				::UnregisterHotKey(m_hWnd, HT_FINDCPUPAGE);
				if (RegisterHotKey(m_hWnd, HT_FINDCPUPAGE, cHotkeyDlg.m_wFindCpu1, cHotkeyDlg.m_wFindCpu2) == FALSE)
				{
					mCtlHotKey.SetHotKey(cHotkeyDlg.m_wFindCpu2, cHotkeyDlg.m_wFindCpu1);
					AfxMessageBox(CString(TEXT("注册快捷键【")) + mCtlHotKey.GetHotKeyName() + TEXT("】失败！原因可能是快捷键与别的程序冲突，请更新设置快捷键"));
				}
			}
		}

		m_wMainPanel1 = cHotkeyDlg.m_wMainPanel1;
		m_wMainPanel2 = cHotkeyDlg.m_wMainPanel2;
		m_wBackBegin1 = cHotkeyDlg.m_wBackBegin1;
		m_wBackBegin2 = cHotkeyDlg.m_wBackBegin2;
		m_wBackEnd1 = cHotkeyDlg.m_wBackEnd1;
		m_wBackEnd2 = cHotkeyDlg.m_wBackEnd2;
		m_wEndFind1 = cHotkeyDlg.m_wEndFind1;
		m_wEndFind2 = cHotkeyDlg.m_wEndFind2;
		m_wFindCpu1 = cHotkeyDlg.m_wFindCpu1;
		m_wFindCpu2 = cHotkeyDlg.m_wFindCpu2;


		if (m_pIFileEngine != NULL) {
			//m_pIFileEngine->SetVarValue(IFileEngine::VT_WORD, TEXT("MainPanel1"), &m_wMainPanel1);
			//m_pIFileEngine->SetVarValue(IFileEngine::VT_WORD, TEXT("MainPanel2"), &m_wMainPanel2);
			//m_pIFileEngine->SetVarValue(IFileEngine::VT_WORD, TEXT("BackBegin1"), &m_wBackBegin1);
			//m_pIFileEngine->SetVarValue(IFileEngine::VT_WORD, TEXT("BackBegin2"), &m_wBackBegin2);
			//m_pIFileEngine->SetVarValue(IFileEngine::VT_WORD, TEXT("BackEnd1"), &m_wBackEnd1);
			//m_pIFileEngine->SetVarValue(IFileEngine::VT_WORD, TEXT("BackEnd2"), &m_wBackEnd2);
			//m_pIFileEngine->SetVarValue(IFileEngine::VT_WORD, TEXT("EndFind1"), &m_wEndFind1);
			//m_pIFileEngine->SetVarValue(IFileEngine::VT_WORD, TEXT("EndFind2"), &m_wEndFind2);
			//m_pIFileEngine->SetVarValue(IFileEngine::VT_WORD, TEXT("FindCpu1"), &m_wFindCpu1);
			//m_pIFileEngine->SetVarValue(IFileEngine::VT_WORD, TEXT("FindCpu2"), &m_wFindCpu2);
		}
	}
}


BOOL CWndWizardDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	m_ToolTip.RelayEvent(pMsg);
	return __super::PreTranslateMessage(pMsg);
}


void CWndWizardDlg::OnBnClickedButtonSendmesage()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pIProcEngine == nullptr || m_pIFileEngine == nullptr || m_pIWndModule == nullptr || m_pIWndPreview == nullptr) {
		CWnd::MessageBox(TEXT("已经来不及了！"));
	} else {
		CWnd::MessageBox(TEXT("还赶得上！"));
		//DWORD dwOldProtect = 0;
		//VirtualProtect(&m_pIProcEngine, sizeof(m_pIProcEngine), PAGE_READONLY, &dwOldProtect);
	}
}


BOOL CWndWizardDlg::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	this->KillTimer(IDT_CHECK);
	return __super::DestroyWindow();
}
