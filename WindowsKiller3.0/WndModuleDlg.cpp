// WndModuleDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndModuleDlg.h"
#include "FileVersion.h"
#include "afxdialogex.h"


// CWndModuleDlg 对话框

#define	TIMEUPDATEPROC	1000

struct DATA {
	BOOL		nfav;
	int 		iSubItem;
	CListCtrl*	pListCtrl;
};

enum ModuleColumn {
	FileName,
	FileType,
	FileSize,
	FileCompany,
	FileDescription,
	FilePath,

	ALLColumn
};

enum ProcessColumn {
	ProcName,
	ProcessId,
	ProcPlatform,
	ProcCPUage,
	ProcUserName,
	ThreadsCount,
	WorkingSet,
	VirtualBytes,

	ProcALLColumn
};

enum ViewStyle {
	ProcMod,
	Process,
	Module,

	ViewStyleALL
};

static	ULONG    g_LockNumber = 0;
static	ULONG    g_IWndModuleNumber = 0;
static	HMODULE	 g_hModule = ::GetModuleHandle(NULL/*TEXT("WndModule.dll")*/);

#define	FV_MODULE_BOOL_EMPTY		TEXT("Module_bIsEmpty")
#define	FV_MODULE_BOOL_ISMAX		TEXT("Module_bIsMax")
#define	FV_MODULE_BOOL_ISICON		TEXT("Module_bIsIconc")
#define	FV_MODULE_BOOL_VISBILE		TEXT("Module_bIsVisble")
#define	FV_MODULE_LONG_TREEPROP 	TEXT("Module_lTreeProp")
#define	FV_MODULE_LONG_VIEWFRAME	TEXT("Module_ViewFrame")
#define	FV_MODULE_LONG_VIEWSTYLE	TEXT("Module_ViewStyle")
#define	FV_MODULE_STRUCT_WRECT		TEXT("Module_strwRect")

IFileEngine*	CWndModuleDlg::m_pIFileEngine = NULL;

#ifdef _DEBUG
CString	szDebug;
#endif

IMPLEMENT_DYNAMIC(CWndModuleDlg, CDialog)

CWndModuleDlg::CWndModuleDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_MODULEVIEW, pParent)
{

	//////////////////////////////////////////////////////////////////////////////
	//	Unknown
	m_Ref = 0;
	g_IWndModuleNumber++;
	//////////////////////////////////////////////////////////////////////////////
	//	IWndModule
	m_pRWndModule = NULL;
	m_dwProcItem = ProcessId;
	m_dwViewFrame = ProcMod;
	m_dwViewStyle = ID_VIEW_REPORT;

	m_SetSize = false;
	m_TreeProp = 0;
	m_nfav = TRUE;
	m_hFwnd = NULL;
	m_nCode = 0;
	m_hIcon = AfxGetApp()->LoadIcon(128/*IDR_MAINFRAME*/);

	HRESULT 		hResult = NULL;

	hResult = DllWndEngine::DllCoCreateObject(CLSID_IWndEngine, IID_IWndEngine, (void**)&m_pIWndEngine);
	if (hResult != S_OK)
	{
		AfxMessageBox(TEXT("Create CLSID_IWndEngine failed!\n"), MB_ICONERROR | MB_TOPMOST);
		ExitProcess(0);
	}
	hResult = DllProcEngine::DllCoCreateObject(CLSID_IProcEngine, IID_IProcEngine, (void**)&m_IProcEngine);
	if (hResult != S_OK)
	{
		AfxMessageBox(TEXT("Create CLSID_IProcEngine failed!\n"), MB_ICONERROR | MB_TOPMOST);
		ExitProcess(0);
	}

}

CWndModuleDlg::~CWndModuleDlg()
{
	for (int i = 0; i < m_ImageIcon.GetSize(); i++)
	{
		DestroyIcon(m_ImageIcon.GetAt(i));
	}
	m_ImageIcon.RemoveAll();
	m_ImageElement.RemoveAll();

	m_pIWndEngine->Release();
}

HRESULT  CWndModuleDlg::QueryInterface(const IID& iid, void **ppv)
{
	if (iid == IID_IUnknown)
	{
		*ppv = (IUnknown *)this;
		((IUnknown *)(*ppv))->AddRef();
	}
	else if (iid == IID_IWndModule)
	{
		*ppv = (IWndModule *)this;
		((IWndModule *)(*ppv))->AddRef();
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG	 CWndModuleDlg::AddRef()
{
	m_Ref++;
	return  (ULONG)m_Ref;
}

ULONG	 CWndModuleDlg::Release()
{
	m_Ref--;
	if (m_Ref == 0) {
		g_IWndModuleNumber--;
		delete this;
		return 0;
	}
	return  (ULONG)m_Ref;
}
bool CWndModuleDlg::UpdateWndModuleInfo(LPCTSTR lpTstr, HWND hFwnd, DWORD dwPid)
{
	CString	szString;

	this->UpdateWinProcessInfo(hFwnd, dwPid);

	if (m_hFwnd != hFwnd || dwPid)
	{
		m_hFwnd = hFwnd;
		m_InvalidDll.RemoveAll();
		m_cListCtrl.DeleteAllItems();

		long nModule = AnalyseModule(lpTstr);

		szString.Format(TEXT("窗口进程有%d个模块。"), nModule);

		m_cStatic.SetWindowText(szString);

		GetWindowThreadProcessId(m_hFwnd, &m_dwRemoteId);

	}
	if (this->IsWindowVisible() == false)
		this->ShowWindow(SW_SHOW);
	//else ::SwitchToThisWindow(m_hWnd,TRUE);;

	return true;
}
HWND CWndModuleDlg::GetWndHandle()
{
	return m_hWnd;
}
bool CWndModuleDlg::CreateWndModule(CWnd* pParentWnd)
{
	return CWndModuleDlg::Create(ATL_MAKEINTRESOURCE(CWndModuleDlg::IDD), pParentWnd) != FALSE;
}
bool CWndModuleDlg::SetRWndModule(RWndModule* pRWndModule)
{
	if (pRWndModule && pRWndModule->OnCheckRWndModule())
	{
		m_pRWndModule = pRWndModule;
		return true;
	}
	return false;
}
bool CWndModuleDlg::SetIFileEngine(IFileEngine*pIFileEngine)
{
	m_pIFileEngine = pIFileEngine;

	if (m_pIFileEngine != NULL)
	{
		bool bIsEmpty = true;
		m_pIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_MODULE_BOOL_EMPTY, &bIsEmpty);
		if (bIsEmpty == true)
		{
			CRect		cRect(0, 0, 0, 0);
			TYPEDATA	cTypeData[] = {
			{ IFileEngine::VT_BOOL,FV_MODULE_BOOL_EMPTY,LPCVOID(false),TD_VALUE },
			{ IFileEngine::VT_BOOL,FV_MODULE_BOOL_ISMAX,LPCVOID(false),TD_VALUE },
			{ IFileEngine::VT_BOOL,FV_MODULE_BOOL_ISICON,LPCVOID(false),TD_VALUE },
			{ IFileEngine::VT_BOOL,FV_MODULE_BOOL_VISBILE,LPCVOID(false),TD_VALUE },
			{ IFileEngine::VT_LONG,FV_MODULE_LONG_TREEPROP,LPCVOID(0),TD_VALUE },
			{ IFileEngine::VT_LONG,FV_MODULE_LONG_VIEWFRAME,LPCVOID(ProcMod),TD_VALUE },
			{ IFileEngine::VT_LONG,FV_MODULE_LONG_VIEWSTYLE,LPCVOID(ID_VIEW_REPORT),TD_VALUE },
			{ IFileEngine::VT_STRUCT,FV_MODULE_STRUCT_WRECT,&cRect,sizeof(CRect) },
			};

			m_pIFileEngine->AddVarValue(cTypeData, sizeof(cTypeData) / sizeof(TYPEDATA));

#ifdef	_DEBUG
			printf("CWndModuleDlg::IFileEngine::bIsEmpty == true\n");
			m_pIFileEngine->OutPutDataInfo();
#endif
		}
	}

	return m_pIFileEngine != NULL;
}
BOOL CWndModuleDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	//ASSERT(pParentWnd != NULL);
	ASSERT(IS_INTRESOURCE(lpszTemplateName) || AfxIsValidString(lpszTemplateName));

	m_lpszTemplateName = lpszTemplateName;  // used for help
	if (IS_INTRESOURCE(m_lpszTemplateName) && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD_PTR)m_lpszTemplateName);

	HINSTANCE hInst = ::GetModuleHandle(NULL/*TEXT("WndModule.dll")*/);
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);

	BOOL bResult = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);
	UnlockResource(hTemplate);
	FreeResource(hTemplate);

	return bResult;
}

void CWndModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_MODULEVIEW, m_cListCtrl);
	DDX_Control(pDX, IDC_LIST_PROCVIEW, m_cListBox);
	DDX_Control(pDX, IDC_STATE, m_cStatic);
}

BEGIN_MESSAGE_MAP(CWndModuleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDCANCEL, &CWndModuleDlg::OnBnClickedButtonOkCancel)
	ON_BN_CLICKED(IDOK, &CWndModuleDlg::OnBnClickedButtonOkCancel)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_MODULEVIEW, &CWndModuleDlg::OnLvnColumnclickListModuleview)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MODULEVIEW, &CWndModuleDlg::OnNMDblclkListModuleview)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_MODULEVIEW, &CWndModuleDlg::OnNMCustomdrawListModuleview)
	ON_WM_SIZING()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_LBN_DBLCLK(IDC_LIST_PROCVIEW, &CWndModuleDlg::OnLbnDblclkListProcview)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PROCVIEW, &CWndModuleDlg::OnNMDblclkListProcview)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_PROCVIEW, &CWndModuleDlg::OnLvnColumnclickListProcview)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CWndModuleDlg 消息处理程序

BOOL CWndModuleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	struct ListHeader {
		LPCTSTR		lpName;
		DWORD		dwSize;
		DWORD		nFormat;
		DWORD		dwColumn;
	}clhc[] = {
	{ TEXT("模块名称"),150,LVCFMT_LEFT,FileName },
	{ TEXT("模块路径"),550,LVCFMT_LEFT,FilePath },
	{ TEXT("类型"),60,LVCFMT_LEFT,FileType },
	{ TEXT("大小"),70,LVCFMT_RIGHT,FileSize },
	{ TEXT("公司"),150,LVCFMT_LEFT,FileCompany },
	{ TEXT("描述"),280,LVCFMT_LEFT,FileDescription },
	}
	, ProcLh[] = {
	{ TEXT("名称"),180,LVCFMT_LEFT,ProcName },
	{ TEXT("PID"),60,LVCFMT_LEFT,ProcessId },
	{ TEXT("平台"),40,LVCFMT_LEFT,ProcPlatform },
	{ TEXT("CPU"),35,LVCFMT_RIGHT,ProcCPUage },
	{ TEXT("用户名"),150,LVCFMT_LEFT,ProcUserName },
	{ TEXT("线程数"),60,LVCFMT_RIGHT,ThreadsCount },
	{ TEXT("内存使用"),120,LVCFMT_RIGHT,WorkingSet },
	{ TEXT("虚拟内存大小"),150,LVCFMT_RIGHT,VirtualBytes },
	};

	m_ImageList16.Create(16, 16, ILC_COLOR24 | ILC_MASK, 1, 1);
	m_ImageList32.Create(32, 32, ILC_COLOR24 | ILC_MASK, 1, 1);

	if (m_pIFileEngine != NULL)
	{
		bool bIsMax = false;
		bool bIsIconc = false;
		bool bIsVisble = false;
		CRect wRect(0, 0, 0, 0);

		m_pIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_MODULE_BOOL_ISMAX, &bIsMax);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_MODULE_BOOL_ISICON, &bIsIconc);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_MODULE_BOOL_VISBILE, &bIsVisble);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_TREEPROP, &m_TreeProp);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWFRAME, &m_dwViewFrame);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWSTYLE, &m_dwViewStyle);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_STRUCT, FV_MODULE_STRUCT_WRECT, &wRect);

		if (bIsMax)
			this->ShowWindow(SW_MAXIMIZE);
		else if (bIsIconc)
			this->ShowWindow(SW_MINIMIZE);
		else if (wRect.Width() && wRect.Height())
			this->MoveWindow(wRect.left, wRect.top, wRect.Width(), wRect.Height());

		ShowWindow(bIsVisble ? SW_SHOW : SW_HIDE);
	}

	DWORD dwIds[] = { ID_VIEW_PROCESS,ID_VIEW_MODULE,ID_VIEW_PROCMOD };
	DWORD dwSty[] = { Process,Module,ProcMod };

	for (int i = 0; i < sizeof(dwIds) / sizeof(DWORD); i++) {
		if (m_dwViewFrame == dwSty[i]) {
			this->OnCommand(MAKEWPARAM(dwIds[i], 0), LPARAM(m_hWnd));
		}
	}

	this->OnCommand(MAKEWPARAM(m_dwViewStyle, 0), LPARAM(m_hWnd));


	for (int i = 0; i < ALLColumn; i++)
		m_cListCtrl.InsertColumn(clhc[i].dwColumn, clhc[i].lpName, clhc[i].nFormat, clhc[i].dwSize);

	m_cListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT |/*LVS_EX_GRIDLINES|*/LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES | LVS_EX_HEADERINALLVIEWS);
	m_cListCtrl.SetImageList(&m_ImageList16, LVSIL_SMALL);
	m_cListCtrl.SetImageList(&m_ImageList32, LVSIL_NORMAL);

	::SetWindowLong(m_hWnd, GWL_STYLE, ::GetWindowLong(m_hWnd, GWL_STYLE) | WS_THICKFRAME);

	SetWindowText(TEXT("进程模块"));

	m_cListBox.SetExtendedStyle(LVS_EX_FULLROWSELECT 
		| LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES 
		| LVS_EX_DOUBLEBUFFER | LVS_EX_SNAPTOGRID
		| LVS_EX_REGIONAL | LVS_EX_BORDERSELECT
		| LVS_EX_MULTIWORKAREAS | LVS_EX_INFOTIP
		| LVS_EX_COLUMNOVERFLOW | LVS_EX_COLUMNSNAPPOINTS
		| LVS_EX_HEADERINALLVIEWS);
	m_cListBox.SetImageList(&m_ImageList16, LVSIL_SMALL);
	m_cListBox.SetImageList(&m_ImageList32, LVSIL_NORMAL);

	for (int i = 0; i < ProcALLColumn; i++)
		m_cListBox.InsertColumn(ProcLh[i].dwColumn, ProcLh[i].lpName, ProcLh[i].nFormat, ProcLh[i].dwSize);

	this->SetTimer(TIMEUPDATEPROC, 500, NULL);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CWndModuleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}
bool CWndModuleDlg::RemoteIdFreeLibrary(LPTSTR szDllName)
{
	try {
		if (m_dwRemoteId == ::GetCurrentProcessId())
			throw TEXT("模块属于本进程！");

		DWORD_PTR dwTemp;
		DWORD	dwWritten, hModule = 0;
		HANDLE	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_dwRemoteId);
		if (!hProcess)throw TEXT("打开进程失败！");

		DWORD	dwSize = (lstrlen(szDllName) + 1) * sizeof(TCHAR);
		HMODULE hkernel32 = GetModuleHandle(TEXT("kernel32.dll"));
#ifdef UNICODE
		LPVOID	pGMH = GetProcAddress(hkernel32, "GetModuleHandleW");
#else
		LPVOID	pGMH = GetProcAddress(hkernel32, "GetModuleHandleA");
#endif // UNICODE
		LPVOID	pFAE = GetProcAddress(hkernel32, "FreeLibraryAndExitThread");
		LPVOID	pDLL = VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);

		if (!pDLL)
		{
			CloseHandle(hProcess);
			throw TEXT("代码注入失败！");
		}

		WriteProcessMemory(hProcess, pDLL, szDllName, dwSize, &dwTemp);

		//GetModuleHandle
		HANDLE	tThread = NULL;
		tThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pGMH, pDLL, 0, &dwWritten);
		WaitForSingleObject(tThread, INFINITE);
		GetExitCodeThread(tThread, &hModule);
		CloseHandle(tThread);

#ifdef _DEBUG
		_tprintf(TEXT("GetModuleHandle(%s) = %ld\n"), szDllName, hModule);
#endif
		//FreeLibraryAndExitThread
		tThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFAE, (LPVOID)hModule, 0, NULL);
		WaitForSingleObject(tThread, INFINITE);
		CloseHandle(tThread);
		//GetModuleHandle
		tThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pGMH, pDLL, 0, &dwWritten);
		WaitForSingleObject(tThread, INFINITE);
		GetExitCodeThread(tThread, &hModule);
		CloseHandle(tThread);

		if (hModule != NULL)
		{
			LPVOID pFL = GetProcAddress(hkernel32, "FreeLibrary");
			for (DWORD hRet = 1, n = 0; n < 50 && hModule != 0 && hRet != 0; n++)
			{
				//FreeLibrary
				tThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFL, (LPVOID)hModule, 0, &dwWritten);
				WaitForSingleObject(tThread, INFINITE);
				GetExitCodeThread(tThread, &hRet);
				CloseHandle(tThread);
				//GetModuleHandle
				tThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pGMH, pDLL, 0, &dwWritten);
				WaitForSingleObject(tThread, INFINITE);
				GetExitCodeThread(tThread, &hModule);
				CloseHandle(tThread);
			}
		}
		VirtualFreeEx(hProcess, pDLL, dwSize, MEM_DECOMMIT);
		CloseHandle(hProcess);

		if (hModule == 0)
		{
			m_InvalidDll.Add(CString(szDllName));
			MessageBox(TEXT("模块成功卸载！"), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
			return true;
		}
		else throw TEXT("");
	}
	catch (LPCTSTR perror)
	{
		CString szError = /*::GetLastErrorInfo*/(perror);
		MessageBox(szError += TEXT("模块卸载失败！"), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
	}
	return false;
}
BOOL CWndModuleDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int	 nID = LOWORD(wParam);
	HWND hCtl = HWND(lParam);
	UINT code = UINT(HIWORD(wParam));

	if (nID == ID_ABOUT)
	{
		CDialog	dlgAbout(100/*IDD_ABOUTBOX*/, this);
		dlgAbout.DoModal();
		return 0;
	}
	else if (nID == ID_MODULE_REFASH)
	{
		this->UpdateWinProcessInfo(NULL);
		m_pIWndEngine->QueryProcess(NULL);
		if (m_pIWndEngine->QueryProcess(m_dwRemoteId) == true)
		{
			m_InvalidDll.RemoveAll();
			m_cListCtrl.DeleteAllItems();

			long nModule = AnalyseModule(m_pIWndEngine->GetStageInfo(IWndEngine::PRO_MODULES));

			CString	cString;
			cString.Format(TEXT("进程有%d个模块。"), nModule);

			m_cStatic.SetWindowText(cString);
		}
		else
		{
			CString	cString = TEXT("刷新进程模块信息失败！");
			cString += m_pIWndEngine->GetStageInfo(IWndEngine::LAST_ERROR);
			m_cStatic.SetWindowText(cString);
		}
	}
	else if (nID == ID_CLOSE)
	{
		FORWARD_WM_SYSCOMMAND(m_hWnd, SC_CLOSE, 0, 0, ::SendMessage);
	}
	else if (nID == ID_VIEW_PROCESS)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_MODULE, ID_VIEW_PROCESS, nID, MF_CHECKED | MF_BYCOMMAND);

		m_dwViewFrame = Process;
		if (m_pIFileEngine != NULL)
		{
			m_pIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWFRAME, &m_dwViewFrame);
		}
		::ShowWindow(m_cListBox.m_hWnd, SW_SHOW);
		::ShowWindow(m_cListCtrl.m_hWnd, SW_HIDE);

		this->UpdateWindowsFrame(m_ClientSize.cx, m_ClientSize.cy);
	}
	else if (nID == ID_VIEW_MODULE)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_MODULE, ID_VIEW_PROCESS, nID, MF_CHECKED | MF_BYCOMMAND);

		m_dwViewFrame = Module;
		if (m_pIFileEngine != NULL)
		{
			m_pIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWFRAME, &m_dwViewFrame);
		}
		::ShowWindow(m_cListBox.m_hWnd, SW_HIDE);
		::ShowWindow(m_cListCtrl.m_hWnd, SW_SHOW);
		this->UpdateWindowsFrame(m_ClientSize.cx, m_ClientSize.cy);

	}
	else if (nID == ID_VIEW_PROCMOD)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_MODULE, ID_VIEW_PROCESS, nID, MF_CHECKED | MF_BYCOMMAND);

		m_dwViewFrame = ProcMod;
		if (m_pIFileEngine != NULL)
		{
			m_pIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWFRAME, &m_dwViewFrame);
		}
		::ShowWindow(m_cListBox.m_hWnd, SW_SHOW);
		::ShowWindow(m_cListCtrl.m_hWnd, SW_SHOW);
		this->UpdateWindowsFrame(m_ClientSize.cx, m_ClientSize.cy);
	}
	else if (nID == ID_VIEW_ICON)
	{

		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_ICON, ID_VIEW_LISTER, nID, MF_CHECKED | MF_BYCOMMAND);

		if (m_pIFileEngine != NULL)
		{
			m_pIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWSTYLE, &nID);
		}
		DWORD dwStyle = m_cListCtrl.GetStyle() & ~(0x0003);

		::SetWindowLong(m_cListCtrl.m_hWnd, GWL_STYLE, dwStyle | LVS_ICON);

	}
	else if (nID == ID_VIEW_SICON)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_ICON, ID_VIEW_LISTER, nID, MF_CHECKED | MF_BYCOMMAND);

		if (m_pIFileEngine != NULL)
		{
			m_pIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWSTYLE, &nID);
		}
		DWORD dwStyle = m_cListCtrl.GetStyle() & ~(0x0003);

		::SetWindowLong(m_cListCtrl.m_hWnd, GWL_STYLE, dwStyle | LVS_SMALLICON);

	}
	else if (nID == ID_VIEW_REPORT)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_ICON, ID_VIEW_LISTER, nID, MF_CHECKED | MF_BYCOMMAND);

		if (m_pIFileEngine != NULL)
		{
			m_pIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWSTYLE, &nID);
		}
		DWORD dwStyle = m_cListCtrl.GetStyle() & ~(0x0003);

		::SetWindowLong(m_cListCtrl.m_hWnd, GWL_STYLE, dwStyle | LVS_REPORT);

	}
	else if (nID == ID_VIEW_LISTER)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_ICON, ID_VIEW_LISTER, nID, MF_CHECKED | MF_BYCOMMAND);

		if (m_pIFileEngine != NULL)
		{
			m_pIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWSTYLE, &nID);
		}
		DWORD dwStyle = m_cListCtrl.GetStyle() & ~(0x0003);

		::SetWindowLong(m_cListCtrl.m_hWnd, GWL_STYLE, dwStyle | LVS_LIST);

	}
	else if (nID == ID_MODULE_OPENPATH)
	{
		TCHAR	command[256], szPath[MAX_PATH];

		CMenu* pMenu = GetMenu()->GetSubMenu(1);

		if (pMenu->GetMenuState(ID_MODULE_UNLOAD, MF_BYCOMMAND) & MF_GRAYED)
		{
			m_cListBox.GetItemText(m_cListBox.GetSelectionMark(), ProcessId, szPath, 256);
			m_pIWndEngine->QueryProcess(DWORD(_tcstol(szPath, NULL, 10)));

			lstrcpy(szPath, m_pIWndEngine->GetStageInfo(IWndEngine::WND_PROCESSPATH));
		}
		else
		{
			m_cListCtrl.GetItemText(m_cListCtrl.GetSelectionMark(), FilePath, szPath, 256);
		}

		wsprintf(command, TEXT("/select,%s"), szPath);

		ShellExecute(NULL, TEXT("open"), TEXT("Explorer.exe"), command, TEXT(""), SW_SHOW);
	}
	else if (nID == ID_MODULE_UNLOAD)
	{
		TCHAR	szDllName[MAX_PATH] = TEXT("NULL");
		m_cListCtrl.GetItemText(m_cListCtrl.GetSelectionMark(), FileName, szDllName, MAX_PATH);

		if (lstrcmp(szDllName, TEXT("NULL")) == 0)
		{
			MessageBox(TEXT("读取项目为空"), TEXT("提示"), MB_ICONWARNING | MB_TOPMOST);
			return CWnd::OnCommand(wParam, lParam);
		}

		INT_PTR len = m_InvalidDll.GetSize();
		if (len != 0)
		{
			CString	szItem = szDllName;
			for (INT_PTR i = 0; i < len; i++)
			{
				if (m_InvalidDll.ElementAt(i) == szItem)
				{
					return CWnd::OnCommand(wParam, lParam);
				}
			}
		}

		if (MessageBox(TEXT("确定要卸载模块吗？"), TEXT("提问"), MB_ICONQUESTION | MB_TOPMOST | MB_YESNO) != IDYES)
		{
			return CWnd::OnCommand(wParam, lParam);
		}

		this->RemoteIdFreeLibrary(szDllName);

	}
	else if (nID == ID_KILLPROC)
	{
		CString szPid = m_cListBox.GetItemText(m_cListBox.GetSelectionMark(), ProcessId);
		if (szPid.IsEmpty() == false)
		{
			if (m_pIWndEngine->QueryProcess(DWORD(_tcstol(szPid, NULL, 10))) == false)
			{
				MessageBox(m_pIWndEngine->GetStageInfo(IWndEngine::LAST_ERROR), TEXT("提示"), MB_ICONWARNING | MB_TOPMOST);
				return CWnd::OnCommand(wParam, lParam);
			}
			if (MessageBox(TEXT("确定要结束进程吗？"), TEXT("提问"), MB_ICONQUESTION | MB_TOPMOST | MB_YESNO) != IDYES)
			{
				return CWnd::OnCommand(wParam, lParam);
			}

			if (m_pIWndEngine->SendFunction(IWndEngine::FUN_KILLPROCESS) == true)
			{
				MessageBox(TEXT("结束进程成功"), TEXT("提示"), MB_ICONWARNING | MB_TOPMOST);
				//FORWARD_WM_COMMAND(m_hWnd,ID_MODULE_REFASH,hCtl,code,::PostMessage);
			}
			else
			{
				MessageBox(m_pIWndEngine->GetStageInfo(IWndEngine::LAST_ERROR), TEXT("提示"), MB_ICONWARNING | MB_TOPMOST);
			}
		}
	}
	else if (nID == ID_MODULE_ATTRIB)
	{
		TCHAR	szPath[MAX_PATH];

		CMenu* pMenu = GetMenu()->GetSubMenu(1);

		if (pMenu->GetMenuState(ID_MODULE_UNLOAD, MF_BYCOMMAND) & MF_GRAYED)
		{
			m_cListBox.GetItemText(m_cListBox.GetSelectionMark(), ProcessId, szPath, 256);
			m_pIWndEngine->QueryProcess(DWORD(_tcstol(szPath, NULL, 10)));
			lstrcpy(szPath, m_pIWndEngine->GetStageInfo(IWndEngine::WND_PROCESSPATH));
		}
		else
		{
			POSITION pos = m_cListCtrl.GetFirstSelectedItemPosition();
			m_cListCtrl.GetItemText(m_cListCtrl.GetNextSelectedItem(pos), FilePath, szPath, MAX_PATH);
		}

		SHELLEXECUTEINFO cExecInfo;

		cExecInfo.cbSize = sizeof(cExecInfo);
		cExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_NO_UI;
		cExecInfo.hwnd = m_hWnd;
		cExecInfo.lpVerb = TEXT("properties");
		cExecInfo.lpFile = szPath;
		cExecInfo.lpParameters = NULL;
		cExecInfo.lpDirectory = NULL;
		cExecInfo.nShow = SW_SHOWNORMAL;
		cExecInfo.hInstApp = 0;
		cExecInfo.lpIDList = 0;

		ShellExecuteEx(&cExecInfo);
	}
	else if (nID == ID_REALTIME || nID == ID_HIGHT || nID == ID_HIGHTSTD
		|| nID == ID_STANDARD || nID == ID_LOWERSTD || nID == ID_LOWER) {

		CString szPid = m_cListBox.GetItemText(m_cListBox.GetSelectionMark(), ProcessId);
		if (szPid.IsEmpty() == false)
		{
			static DWORD MID[] = {
				ID_REALTIME,ID_HIGHT,ID_HIGHTSTD,
				ID_STANDARD,ID_LOWERSTD,ID_LOWER
			};
			static DWORD PCL[] = {
				IWndEngine::PRIORITY_CLASS_REALTIME,
				IWndEngine::PRIORITY_CLASS_HIGH,
				IWndEngine::PRIORITY_CLASS_NORMAL_ABOVE,
				IWndEngine::PRIORITY_CLASS_NORMAL,
				IWndEngine::PRIORITY_CLASS_NORMAL_BELOW,
				IWndEngine::PRIORITY_CLASS_IDLE,
				PROCESS_MODE_BACKGROUND_BEGIN,
				PROCESS_MODE_BACKGROUND_END,
			};

			for (int i = 0; i < sizeof(MID) / sizeof(DWORD); i++)
			{
				if (nID == MID[i]) {
					if (false == m_pIWndEngine->SetPriorityClass(DWORD(_tcstol(szPid, NULL, 10)), PCL[i])) {
						MessageBox(m_pIWndEngine->GetStageInfo(IWndEngine::LAST_ERROR), TEXT("提示"), MB_ICONWARNING | MB_TOPMOST);
					}
					break;
				}
			}
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}

void CWndModuleDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: 在此处添加消息处理程序代码
	if (pWnd->m_hWnd == m_cListCtrl.m_hWnd)
	{
		POINT	pt = { point.x,point.y };
		::ScreenToClient(m_cListCtrl.m_hWnd, &pt);

		int index = m_cListCtrl.GetSelectionMark();

		RECT	rc;
		m_cListCtrl.GetItemRect(index, &rc, 0);

		//HDC hdc = ::GetDC(pWnd->m_hWnd);
		//::FillRect(hdc,&rc,GetStockBrush(BLACK_BRUSH));
		//::ReleaseDC(pWnd->m_hWnd,hdc);

		if (PtInRect(&rc, pt))
		{
			CMenu* pMenu = GetMenu()->GetSubMenu(1);
			pMenu->EnableMenuItem(ID_MODULE_UNLOAD, MF_ENABLED);
			pMenu->EnableMenuItem(ID_KILLPROC, MF_GRAYED);

			CMenu* pMenuPriorityClass = pMenu->GetSubMenu(pMenu->GetMenuItemCount() - 1);

			for (int i = 0, id = 0; i < pMenuPriorityClass->GetMenuItemCount(); i++)
			{
				id = pMenuPriorityClass->GetMenuItemID(i);
				pMenu->EnableMenuItem(id, MF_GRAYED);
			}

			pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
		}
	}
	else if (pWnd->m_hWnd == m_cListBox.m_hWnd)
	{
		RECT	rc;
		POINT	pt = { point.x,point.y };
		int index = m_cListBox.GetSelectionMark();

		::ScreenToClient(m_cListBox.m_hWnd, &pt);
		m_cListBox.GetItemRect(index, &rc, 0);


		if (PtInRect(&rc, pt))
		{
			static DWORD mId, MID[] = {
				ID_REALTIME,ID_HIGHT,ID_HIGHTSTD,
				ID_STANDARD,ID_LOWERSTD,ID_LOWER
			};
			static DWORD pCl, PCL[] = {
				IWndEngine::PRIORITY_CLASS_REALTIME,
				IWndEngine::PRIORITY_CLASS_HIGH,
				IWndEngine::PRIORITY_CLASS_NORMAL_ABOVE,
				IWndEngine::PRIORITY_CLASS_NORMAL,
				IWndEngine::PRIORITY_CLASS_NORMAL_BELOW,
				IWndEngine::PRIORITY_CLASS_IDLE,
				PROCESS_MODE_BACKGROUND_BEGIN,
				PROCESS_MODE_BACKGROUND_END,
			};
			CMenu* pMenu = GetMenu()->GetSubMenu(1);
			pMenu->EnableMenuItem(ID_KILLPROC, MF_ENABLED);
			pMenu->EnableMenuItem(ID_MODULE_UNLOAD, MF_GRAYED);

			CString szPid = m_cListBox.GetItemText(index, ProcessId);
			m_pIWndEngine->GetPriorityClass(DWORD(_tcstol(szPid, NULL, 10)), &pCl);

			for (int i = 0; i < sizeof(MID) / sizeof(DWORD); i++) {
				if (pCl == PCL[i]) {
					mId = MID[i];
					break;
				}
			}

			CMenu* pMenuPriorityClass = pMenu->GetSubMenu(pMenu->GetMenuItemCount() - 1);

			for (int i = 0, id = 0; i < pMenuPriorityClass->GetMenuItemCount(); i++)
			{
				id = pMenuPriorityClass->GetMenuItemID(i);
				if (id == mId) {
					pMenuPriorityClass->CheckMenuItem(id, MF_CHECKED | MF_BYCOMMAND);
				}
				else {
					pMenuPriorityClass->CheckMenuItem(id, MF_UNCHECKED | MF_BYCOMMAND);
				}
				pMenu->EnableMenuItem(id, MF_ENABLED);
			}

			pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
		}
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWndModuleDlg::OnPaint()
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
HCURSOR CWndModuleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CWndModuleDlg::OnClose()
{
	CDialog::OnClose();
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	this->ShowWindow(SW_HIDE);
	//this->GetParent()->SetActiveWindow();
}
void CWndModuleDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialog::OnMouseMove(nFlags, point);
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_dwViewFrame == ProcMod)
	{
		int		iPos = int(m_TreeProp);
		bool	bin = point.x > iPos - 1 && point.x < iPos + 6 && point.y < m_ClientSize.cy - 15;

		if (m_hWnd != ::GetCapture())
		{
			if (bin == true)
			{
				::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
				::SetCapture(m_hWnd);
			}
		}
		else
		{
			if (m_SetSize == true)
			{
				int cx = m_ClientSize.cx;
				int cy = m_ClientSize.cy;

				if (point.x < 2)point.x = 2;
				else if (point.x > cx - 5)point.x = cx - 5;

				//::MoveWindow(m_cListBox.m_hWnd, 0, 0, point.x, cy - 18, TRUE);
				//::MoveWindow(m_cListCtrl.m_hWnd, point.x + 3, 0, cx - point.x - 3, cy - 18, TRUE);

				m_TreeProp = point.x;

				this->UpdateWindowsFrame(cx, cy);

				if (m_pIFileEngine != NULL)
				{
					m_pIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_TREEPROP, &m_TreeProp);
				}
			}
			else if (bin == false)
			{
				::ReleaseCapture();
			}
		}
	}

}

void CWndModuleDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: 在此处添加消息处理程序代码

	m_ClientSize.cx = cx;
	m_ClientSize.cy = cy;
	
	if (FALSE == ::IsWindow(m_cStatic.m_hWnd))
	{
		m_ClientSize.cy -= 3;
	}

	this->UpdateWindowsFrame(cx,cy);

}


void CWndModuleDlg::OnBnClickedButtonOkCancel()
{
	this->ShowWindow(SW_HIDE);
	//this->GetParent()->SetActiveWindow();
}
void CWndModuleDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	// TODO: 在此处添加消息处理程序代码
	if (bShow)this->CenterWindow();
}

void CWndModuleDlg::OnLbnDblclkListProcview()
{
	// TODO: 在此添加控件通知处理程序代码

	CString	szText, cString;

	szText = m_cListBox.GetItemText(m_cListBox.GetSelectionMark(), ProcessId);

	//m_dwRemoteId = DWORD(_tcstol(szText,NULL,16));
	m_dwRemoteId = DWORD(_tcstol(szText, NULL, 10));

	if (m_pIWndEngine->QueryProcess(m_dwRemoteId) == true)
	{
		m_InvalidDll.RemoveAll();
		m_cListCtrl.DeleteAllItems();

		long nModule = AnalyseModule(m_pIWndEngine->GetStageInfo(IWndEngine::PRO_MODULES));

		int iF = szText.Find('|');

		szText = szText.Right(szText.GetLength() - iF - 1);
		cString.Format(TEXT("进程%s有%d个模块。"), LPCTSTR(szText), nModule);

		m_cStatic.SetWindowText(cString);
	}

}
long CWndModuleDlg::AnalyseModule(LPCTSTR lpStr)
{
	CString	cString = lpStr, szModPath, szModule;
	CFileVersion	cFileVersion;

	TCHAR	szFile[MAX_PATH] = TEXT("[NULL]"), szType[MAX_PATH] = TEXT("[NULL]");

	int iCode = 0;
	for (int i = 0, j = 0, n = 0, code; (i = cString.Find(TEXT("\r\n"), n)) != -1; n = i + 2)
	{
		if (i > n)
		{
			szModPath = cString.Mid(n, i - n);
			_tsplitpath_s(szModPath, 0, 0, 0, 0, szFile, MAX_PATH, szType, MAX_PATH);
			szModule.Format(TEXT("%s%s"), szFile, szType);

			code = GetTypeIconCode(szType, szModPath);

			iCode++;
			cFileVersion.SetFilePath(szModPath);
			m_cListCtrl.InsertItem(j, NULL, code);
			m_cListCtrl.SetItemText(j, FileName, szModule);
			m_cListCtrl.SetItemText(j, FileType, szType);
			m_cListCtrl.SetItemText(j, FilePath, szModPath);
			m_cListCtrl.SetItemText(j, FileCompany, cFileVersion.GetCompanyName());
			m_cListCtrl.SetItemText(j, FileDescription, cFileVersion.GetFileDescription());

			HANDLE handle = CreateFile(szModPath, FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
			if (handle != INVALID_HANDLE_VALUE)
			{
				DWORD fSize = GetFileSize(handle, NULL);
				CloseHandle(handle);
				//TCHAR szSize[32];
				//float ss = float(fSize) / 1024.0;
				//if (fSize > 1048576)
				//	wsprintf(szSize, TEXT("%.2fMB"), float(fSize) / 1048576.0);
				//else if (fSize > 1024)
				//	wsprintf(szSize, TEXT("%.2fKB"), float(fSize) / 1024.0);
				//else
				//	wsprintf(szSize, TEXT("%字节"), fSize);

				CString szSize;
				if (fSize > 1048576)
					szSize.Format(TEXT("%.2fMB"), float(fSize) / 1048576.0);
				else if (fSize > 1024)
					szSize.Format(TEXT("%.2fKB"), float(fSize) / 1024.0);
				else
					szSize.Format(TEXT("%字节"), fSize);
				m_cListCtrl.SetItemText(j, FileSize, szSize);
			}
			else
			{
				m_cListCtrl.SetItemText(j, FileSize, TEXT("错误"));
			}
		}
	}

	return iCode;
}
void CWndModuleDlg::UpdateWinProcessInfo(HWND hwnd, DWORD dwPid)
{
	if (m_IProcEngine->UpdateProcessInfo())
	{
		DWORD	dwWndPid = dwPid;
		if (dwWndPid == 0)
		{
			::GetWindowThreadProcessId(hwnd, &dwWndPid);
		}

		CString	szString;
		m_cListBox.DeleteAllItems();
		while (LPPROCCONFIG lpConfig = m_IProcEngine->GetProcessConfig())
		{
			m_pIWndEngine->QueryProcess(lpConfig->dwProcessID);
			m_cListBox.InsertItem(0, NULL, GetTypeIconCode(TEXT(".exe"), m_pIWndEngine->GetStageInfo(IWndEngine::WND_PROCESSPATH)));

			//szString.Format(TEXT("%#08x|%d"),lpConfig->dwProcessID,lpConfig->dwProcessID);
			szString.Format(TEXT("%04d"), lpConfig->dwProcessID);
			m_cListBox.SetItemText(0, ProcessId, szString);

			m_cListBox.SetItemText(0, ProcName, lpConfig->szProcessName);
			m_cListBox.SetItemText(0, ProcUserName, lpConfig->szUserName);

			szString.Format(TEXT("%02d"), lpConfig->ulCPUPage);
			m_cListBox.SetItemText(0, ProcCPUage, szString);
			m_cListBox.SetItemText(0, ProcPlatform, lpConfig->blIsWin32 ? TEXT("x86") : TEXT("x64"));

			szString.Format(TEXT("%d"), lpConfig->dwThreadsCount);
			m_cListBox.SetItemText(0, ThreadsCount, szString);

			int temp = 0;
			if ((temp = lpConfig->dwWorkingSet / (1000 * 1000)) != 0)
				szString.Format(TEXT("%d,%03dK"), temp, (lpConfig->dwWorkingSet % (1000 * 1000)) / 1000);
			else
				szString.Format(TEXT("%dK"), lpConfig->dwWorkingSet / 1000);

			m_cListBox.SetItemText(0, WorkingSet, szString);

			if ((temp = lpConfig->dwVirtualBytes / (1000 * 1000)) != 0)
				szString.Format(TEXT("%d,%03dK"), temp, (lpConfig->dwVirtualBytes % (1000 * 1000)) / 1000);
			else
				szString.Format(TEXT("%dK"), lpConfig->dwVirtualBytes / 1000);

			m_cListBox.SetItemText(0, VirtualBytes, szString);

			if (dwWndPid == lpConfig->dwProcessID)
			{
				m_cListBox.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				//m_cListBox.SetItemState(0,0xFFFFFFFF,LVIS_SELECTED); 
			}
		}
	}
}

void CWndModuleDlg::UpdateWindowsFrame(int cx, int cy)
{
	int	iPos = int(m_TreeProp);

	if (m_dwViewFrame == ProcMod)
	{
		::MoveWindow(m_cListBox.m_hWnd, 0, 1, iPos, cy - 23, TRUE);
		::MoveWindow(m_cListCtrl.m_hWnd, iPos + 3, 1, cx - iPos - 3, cy - 23, TRUE);
	}
	else if (m_dwViewFrame == Process)
	{
		::MoveWindow(m_cListBox.m_hWnd, 3, 1, cx - 6, cy - 23, TRUE);
	}
	else if (m_dwViewFrame == Module)
	{
		::MoveWindow(m_cListCtrl.m_hWnd, 3, 1, cx - 6, cy - 23, TRUE);
	}
	::MoveWindow(m_cStatic.m_hWnd, 0, cy - 19, cx, 19, TRUE);
}


DWORD CWndModuleDlg::AddTypeIconCode(LPCTSTR lpStr, LPCTSTR lpFileName)
{
	DWORD	dwCode = 0;
	HICON	hIcon = NULL;

	SHFILEINFO   sfi;
	SHGetFileInfo(lpStr, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
	hIcon = sfi.hIcon;

	m_ImageElement.Add(ImageListElement(lpStr, dwCode = m_nCode++));

	m_ImageIcon.Add(hIcon);
	m_ImageList16.Add(hIcon);
	m_ImageList32.Add(hIcon);

	return dwCode;
}
DWORD	CWndModuleDlg::GetTypeIconCode(LPCTSTR lpStr)
{
#ifdef _DEBUG
	CString	szTemp;
	szTemp.Format(TEXT("CWndModuleDlg::GetTypeIconCode(%s)\n"), lpStr);
	szDebug += szTemp;
#endif
	for (int i = 0; i < m_ImageElement.GetSize(); i++)
	{
#ifdef _DEBUG
		ImageListElement &ListElement = m_ImageElement.ElementAt(i);
		szTemp.Format(TEXT("    [%02d] = %s\n"), i, LPCTSTR(ListElement.szType));
		szDebug += szTemp;
		if (ListElement.szType.CompareNoCase(lpStr) == 0)
		{
			szTemp.Format(TEXT("CWndModuleDlg::GetTypeIconCode::Find[%02d]!\n"), ListElement.dwCode);
			szDebug += szTemp;
			return ListElement.dwCode;
		}
#else
		if (m_ImageElement.ElementAt(i).szType.CompareNoCase(lpStr) == 0)
		{
			return m_ImageElement.ElementAt(i).dwCode;
		}
#endif
	}
#ifdef _DEBUG
	szTemp.Format(TEXT("CWndModuleDlg::GetTypeIconCode::NotFind!\n"));
	szDebug += szTemp;
#endif
	return -1;
}
DWORD	CWndModuleDlg::AddFileIconCode(LPCTSTR lpFileName)
{
	HICON	hIcon = ExtractIcon(::GetModuleHandle(NULL), lpFileName, 0);

#ifdef _DEBUG
	CString	szTemp;
	szTemp.Format(TEXT("CWndModuleDlg::AddFileIconCode::(%s) = %#x\n"), lpFileName, hIcon);
	szDebug += szTemp;
#endif

	if (hIcon != NULL)
	{
		m_ImageIcon.Add(hIcon);
		m_ImageList16.Add(hIcon);
		m_ImageList32.Add(hIcon);
		m_ImageElement.Add(ImageListElement(lpFileName, m_nCode));

		return m_nCode++;
	}
	return -1;
}
DWORD	CWndModuleDlg::GetTypeIconCode(LPCTSTR lpExt, LPCTSTR lpFileName)
{
#ifdef _DEBUG
	CString	szTemp;
	szDebug.Format(TEXT("CWndModuleDlg::GetTypeIconCode(%s,%s)\n"), lpExt, lpFileName);
#endif

	DWORD dwCode = GetTypeIconCode(lpExt);

	if (dwCode == -1)
	{
		dwCode = AddTypeIconCode(lpExt, lpFileName);
	}

	if (lstrcmpi(lpExt, TEXT(".exe")) == 0)
	{
		DWORD dwFile = GetTypeIconCode(lpFileName);
		if (dwFile == -1)
		{
#ifdef _DEBUG
			szDebug += TEXT("\t::GetTypeIconCode::GetIcon(lpFileName) = -1\n");
#endif
			dwFile = AddFileIconCode(lpFileName);
			if (dwFile == -1)
				return dwCode;
			dwCode = dwFile;
		}

#ifdef _DEBUG
		else
		{
			szTemp.Format(TEXT("CWndModuleDlg::GetTypeIconCode::GetIcon = %d\n"), dwFile); \
				szDebug += szTemp;
			return dwFile;
		}
#else
		else return dwFile;
#endif
	}

	return dwCode;
}
void CWndModuleDlg::OnNMDblclkListModuleview(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码

	TCHAR	szPath[MAX_PATH];

	int index = m_cListCtrl.GetSelectionMark();

	m_cListCtrl.GetItemText(index, FilePath, szPath, 256);

	ShellExecute(NULL, TEXT("open"), szPath, TEXT(""), TEXT(""), SW_SHOW);

#ifdef _DEBUG
	_tprintf(TEXT("CWndModuleDlg::%s%s"), szPath, ::GetLastErrorInfo());
#endif

	*pResult = 0;
}

void CWndModuleDlg::OnNMCustomdrawListModuleview(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{//这里可以编写代码设置某一行的颜色

		INT_PTR len = m_InvalidDll.GetSize();
		if (len != 0)
		{
			int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);
			CString	szItem = m_cListCtrl.GetItemText(int(nItem), FileName);
			for (INT_PTR i = 0; i < len; i++)
			{
				if (m_InvalidDll.ElementAt(i) == szItem)
				{
					pLVCD->clrText = RGB(255, 0, 0);
					break;
				}
			}
		}

		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		return;
	}
	else if ((CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage)
	{//这里可以设定具体某一行某一格的背景颜色
		int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);
		int nSubItem = pLVCD->iSubItem;
		if (nSubItem == 1 && nItem == 1)
		{
			//pLVCD->clrText = RGB(0,100,0);
		}
	}

	*pResult = 0;
}

void CWndModuleDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_dwViewFrame == ProcMod && m_hWnd == ::GetCapture())
	{
		m_SetSize = true;
	}
}

void CWndModuleDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonUp(nFlags, point);
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_dwViewFrame == ProcMod && m_hWnd == ::GetCapture() || m_SetSize == true)
	{
		m_SetSize = false;
		::ReleaseCapture();
	}
}

void CWndModuleDlg::OnNMDblclkListProcview(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	CString	cString;
	CString	szText = m_cListBox.GetItemText(m_cListBox.GetSelectionMark(), ProcessId);
	CString	szProc = m_cListBox.GetItemText(m_cListBox.GetSelectionMark(), ProcName);

	if (this->m_dwViewFrame != ProcMod)
	{
		FORWARD_WM_COMMAND(m_hWnd, ID_VIEW_PROCMOD, m_hWnd, 0, ::SendMessage);
	}

	//m_dwRemoteId = DWORD(_tcstol(szText,NULL,16));
	m_dwRemoteId = DWORD(_tcstol(szText, NULL, 10));

	m_InvalidDll.RemoveAll();
	m_cListCtrl.DeleteAllItems();

	long nModule = 0;

	if (m_pIWndEngine->QueryProcess(m_dwRemoteId) == true)
	{
		m_cListCtrl.SetRedraw(FALSE);

		nModule = AnalyseModule(m_pIWndEngine->GetStageInfo(IWndEngine::PRO_MODULES));
		//更新内容
		m_cListCtrl.SetRedraw(TRUE);
		m_cListCtrl.Invalidate();
		m_cListCtrl.UpdateWindow();

	}

	szText = szText.Right(szText.GetLength() - szText.Find('|') - 1);
	cString.Format(TEXT("进程%s有%d个模块。"), LPCTSTR(szProc), nModule);

	m_cStatic.SetWindowText(cString);

	*pResult = 0;
}

void CWndModuleDlg::OnLvnColumnclickListProcview(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码


	for (int i = 0; i < m_cListBox.GetItemCount(); ++i) //这个for用来给表中的项添加上索引号
	{
		m_cListBox.SetItemData(i, i);
	}

	m_nfav = !m_nfav;

	DATA data = { m_nfav,m_dwProcItem = pNMLV->iSubItem,&m_cListBox };

	m_cListBox.SortItems(ProcCompare, (LPARAM)&data);

	*pResult = 0;
}

void CWndModuleDlg::OnLvnColumnclickListModuleview(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	for (int i = 0; i < m_cListCtrl.GetItemCount(); ++i) //这个for用来给表中的项添加上索引号
	{
		m_cListCtrl.SetItemData(i, i);
	}

	m_nfav = !m_nfav;

	DATA data = { m_nfav,pNMLV->iSubItem,&m_cListCtrl };

	m_cListCtrl.SortItems(listCompare, (LPARAM)&data);

	*pResult = 0;
}
int	CALLBACK CWndModuleDlg::ProcCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	DATA* pData = (DATA*)lParamSort;

	int col = pData->iSubItem;//点击的列项传递给col，用来判断点击了第几列

	float cmp = 0;
	if (col == ThreadsCount || col == WorkingSet || col == VirtualBytes)
	{
		CString	szSize1 = (pData->pListCtrl)->GetItemText(int(lParam1), col);
		CString	szSize2 = (pData->pListCtrl)->GetItemText(int(lParam2), col);

		cmp = float(szSize1.GetLength() - szSize2.GetLength());

		if (cmp == 0)
			cmp = (float)lstrcmp((pData->pListCtrl)->GetItemText(int(lParam1), col), (pData->pListCtrl)->GetItemText(int(lParam2), col));
	}
	else
		cmp = (float)lstrcmp((pData->pListCtrl)->GetItemText(int(lParam1), col), (pData->pListCtrl)->GetItemText(int(lParam2), col));

	if (cmp > 0)return 1 - pData->nfav * 2;
	else if (cmp < 0)return pData->nfav * 2 - 1;
	else return int(cmp);

	return 0;
}

int	CALLBACK CWndModuleDlg::listCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	DATA* pData = (DATA*)lParamSort;

	int col = pData->iSubItem;//点击的列项传递给col，用来判断点击了第几列

	float cmp = 0;
	if (col == FileSize)
	{
		CString	szSize1 = (pData->pListCtrl)->GetItemText(int(lParam1), col);
		CString	szSize2 = (pData->pListCtrl)->GetItemText(int(lParam2), col);

		cmp = float(szSize1[szSize1.GetLength() - 2] - szSize2[szSize2.GetLength() - 2]);

		
		if (cmp == 0)
			cmp = float(_tstof(szSize1) - _tstof(szSize2));
	}
	else
		cmp = (float)lstrcmp((pData->pListCtrl)->GetItemText(int(lParam1), col), (pData->pListCtrl)->GetItemText(int(lParam2), col));

	if (cmp > 0)return 1 - pData->nfav * 2;
	else if (cmp < 0)return pData->nfav * 2 - 1;
	else return int(cmp);

	return 0;
}


void CWndModuleDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);

	// TODO: 在此处添加消息处理程序代码

	if (m_pIFileEngine != NULL)
	{
		m_pIFileEngine->SetVarValue(IFileEngine::VT_STRUCT, FV_MODULE_STRUCT_WRECT, pRect);
	}
}

void CWndModuleDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//#ifdef _DEBUG
	long procount = m_IProcEngine->UpdateProcessInfo();
	if (TIMEUPDATEPROC == nIDEvent && IsWindowVisible() && !IsIconic() && procount)
		/*
		#else
		if(TIMEUPDATEPROC == nIDEvent  && IsWindowVisible() && !IsIconic() && m_IProcEngine->UpdateProcessInfo())
		#endif
		*/
	{

		static struct PROCCONFIG {
			ULONG	ulCPUPage;
			DWORD	dwThreadsCount;		//线程数目
			DWORD	dwProcessID;		//进程标识符；
			DWORD	dwVirtualBytes;		//虚拟存储大小；	
			DWORD	dwWorkingSet;		//工作集大小；
			TCHAR	szProcName[64];
			TCHAR	szUserName[64];
		}ProcConfig[200];

		int	nProcess = 0;
#ifdef _DEBUG
		static	int nCount = 0;
		nCount++;
#endif
		while (LPPROCCONFIG lpConfig = m_IProcEngine->GetProcessConfig())
		{
			ProcConfig[nProcess].ulCPUPage = lpConfig->ulCPUPage;
			ProcConfig[nProcess].dwProcessID = lpConfig->dwProcessID;
			ProcConfig[nProcess].dwThreadsCount = lpConfig->dwThreadsCount;
			ProcConfig[nProcess].dwVirtualBytes = lpConfig->dwVirtualBytes;
			ProcConfig[nProcess].dwWorkingSet = lpConfig->dwWorkingSet;
			lstrcpyn(ProcConfig[nProcess].szUserName, lpConfig->szUserName, 64);
			lstrcpyn(ProcConfig[nProcess].szProcName, lpConfig->szProcessName, 64);
#ifdef _DEBUG
			//printf("%-25s%8d%4d\n"),lpConfig->szProcessName,lpConfig->dwProcessID,lpConfig->ulCPUPage);
#endif
			nProcess++;
		}


		DWORD dwPid = 0;
		CString	strFormat;
		for (int n, i = 0, len = m_cListBox.GetItemCount(); i < len; i++)
		{
			//dwPid = DWORD(_tcstol(m_cListBox.GetItemText(i,ProcessId),NULL,16));
			dwPid = DWORD(_tcstol(m_cListBox.GetItemText(i, ProcessId), NULL, 10));
			for (n = i; n < nProcess; n++)
			{
				if (ProcConfig[n].dwProcessID == dwPid)
				{
					int temp = 0;
					strFormat.Format(TEXT("%02d"), ProcConfig[n].ulCPUPage);
					m_cListBox.SetItemText(i, ProcCPUage, strFormat);

					strFormat.Format(TEXT("%d"), ProcConfig[n].dwThreadsCount);
					m_cListBox.SetItemText(i, ThreadsCount, strFormat);

					if ((temp = ProcConfig[n].dwWorkingSet / (1000 * 1000)) != 0)
						strFormat.Format(TEXT("%d,%03dK"), temp, (ProcConfig[n].dwWorkingSet % (1000 * 1000)) / 1000);
					else
						strFormat.Format(TEXT("%dK"), ProcConfig[n].dwWorkingSet / 1000);

					m_cListBox.SetItemText(i, WorkingSet, strFormat);

					if ((temp = ProcConfig[n].dwVirtualBytes / (1000 * 1000)) != 0)
						strFormat.Format(TEXT("%d,%03dK"), temp, (ProcConfig[n].dwVirtualBytes % (1000 * 1000)) / 1000);
					else
						strFormat.Format(TEXT("%dK"), ProcConfig[n].dwVirtualBytes / 1000);
					m_cListBox.SetItemText(i, VirtualBytes, strFormat);

					if (n != i)
					{
						ProcConfig[nProcess] = ProcConfig[n];
						ProcConfig[n] = ProcConfig[i];
						ProcConfig[i] = ProcConfig[nProcess];
					}
					break;
				}
			}
			if (n == nProcess && i < nProcess)//n == nProcess ：：找不到 ；i < nProcess
			{
#ifdef _DEBUG
				_tprintf(TEXT("CWndModuleDlg::OnTimer::DeleteItem::%04d = %s[procount = %d]\n"), nCount, (LPCTSTR)(m_cListBox.GetItemText(i, ProcName)), procount);
#endif
				m_cListBox.DeleteItem(i);
				if (i < len - 1)i--, len--;
			}
		}
		if (m_dwProcItem == ThreadsCount || m_dwProcItem == WorkingSet || m_dwProcItem == VirtualBytes || m_dwProcItem == ProcCPUage)
		{
			for (int i = 0; i < m_cListBox.GetItemCount(); ++i) //这个for用来给表中的项添加上索引号
			{
				m_cListBox.SetItemData(i, i);
			}
			DATA data = { m_nfav,m_dwProcItem,&m_cListBox };
			m_cListBox.SortItems(ProcCompare, (LPARAM)&data);
		}


		for (int len = m_cListBox.GetItemCount(); len < nProcess; len++)
		{
#ifdef _DEBUG
			static int addCount = 0;
			addCount++;
#endif
			if (m_pIWndEngine->QueryProcess(ProcConfig[len].dwProcessID) != false)
			{
#ifdef _DEBUG
				LPCTSTR lpPath = m_pIWndEngine->GetStageInfo(IWndEngine::WND_PROCESSPATH);
				int nImage = GetTypeIconCode(TEXT(".exe"), lpPath);
				if (nImage == 0 && ExtractIcon(::GetModuleHandle(NULL), lpPath, 0) != NULL)
				{
					_tsystem(TEXT("cls"));
					_tprintf(TEXT("CWndModuleDlg::GetTypeIconCode::Error !\n"));
					_tprintf(LPCTSTR(szDebug));
				}
				m_cListBox.InsertItem(0, NULL, nImage);
#else
				m_cListBox.InsertItem(0, NULL, GetTypeIconCode(TEXT(".exe"), m_pIWndEngine->GetStageInfo(IWndEngine::WND_PROCESSPATH)));
#endif
			}
			else m_cListBox.InsertItem(0, NULL, 0);


			//strFormat.Format(TEXT("%#08x|%d"),ProcConfig[len].dwProcessID,ProcConfig[len].dwProcessID);
			strFormat.Format(TEXT("%04d"), ProcConfig[len].dwProcessID);
			m_cListBox.SetItemText(0, ProcessId, strFormat);

			m_cListBox.SetItemText(0, ProcName, ProcConfig[len].szProcName);
			m_cListBox.SetItemText(0, ProcUserName, ProcConfig[len].szUserName);
#ifdef _DEBUG
			_tprintf(TEXT("CWndModuleDlg::OnTimer::InsertItem::%04d = %s[procount = %d]\n"), nCount, (LPCTSTR)(m_cListBox.GetItemText(0, ProcName)), procount);
#endif
			strFormat.Format(TEXT("%d"), ProcConfig[len].ulCPUPage);
			m_cListBox.SetItemText(0, ProcCPUage, strFormat);

			strFormat.Format(TEXT("%d"), ProcConfig[len].dwThreadsCount);
			m_cListBox.SetItemText(0, ThreadsCount, strFormat);

			int temp = 0;
			if ((temp = ProcConfig[len].dwWorkingSet / (1000 * 1000)) != 0)
				strFormat.Format(TEXT("%d,%dK"), temp, (ProcConfig[len].dwWorkingSet % (1000 * 1000)) / 1000);
			else
				strFormat.Format(TEXT("%dK"), ProcConfig[len].dwWorkingSet / 1000);

			m_cListBox.SetItemText(0, WorkingSet, strFormat);

			if ((temp = ProcConfig[len].dwVirtualBytes / (1000 * 1000)) != 0)
				strFormat.Format(TEXT("%d,%dK"), temp, (ProcConfig[len].dwVirtualBytes % (1000 * 1000)) / 1000);
			else
				strFormat.Format(TEXT("%dK"), ProcConfig[len].dwVirtualBytes / 1000);

			m_cListBox.SetItemText(0, VirtualBytes, strFormat);
		}
	}
	__super::OnTimer(nIDEvent);
}


LPVOID CWndModuleDlg::GetProcEngine()
{
	return LPVOID(m_IProcEngine);
}
