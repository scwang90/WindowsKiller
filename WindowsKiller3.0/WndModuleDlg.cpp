// WndModuleDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndModuleDlg.h"
#include "afxdialogex.h"


// CWndModuleDlg 对话框

#define	TIME_UPDATE_PROC	1000
#define SPACE_SIZE_UNIT		1024

struct DATA {
	BOOL		nfav;
	int 		iSubItem;
	CListCtrl*	pListCtrl;
};

//typedef struct LvColumn {
//	LPCTSTR		lpName;
//	DWORD		dwSize;
//	DWORD		nFormat;
//}*LpLvColumn;
//
//enum class LvColumnModule : INT_PTR {
//	Name = &LvColumn = {}
//};

enum ModuleColumn {
	FileName,
	FileType,
	FileSize,
	FileCompany,
	FileDescription,
	FilePath,

	FileALLColumn
};

enum ProcessColumn {
	ProcName,
	ProcessId,
	ProcCPUage,
	ProcThreads,
	ProcWorkingSet,
	ProcUserName,
	//VirtualBytes,
	ProcCreateTime,
	ProcRunTime,
	ProcPlatform,
	ProcCompany,
	ProcDescription,
	ProcPath,

	ProcALLColumn
};

enum ViewStyle {
	ProcMod,
	ViewProcess,
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
#define	FV_MODULE_STRING_INJECT		TEXT("Module_strInject")

IFileEngine*	CWndModuleDlg::msPtrIFileEngine = NULL;

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
	mPtrRWndModule = NULL;
	m_dwProcItem = ProcessId;
	m_dwViewFrame = ProcMod;
	m_dwViewStyle = ID_VIEW_REPORT;

	m_SetSize = false;
	m_TreeProp = 0;
	m_nfav = TRUE;
	m_hFwnd = NULL;
	m_nCode = 0;
	m_hIcon = AfxGetApp()->LoadIcon(128/*IDR_MAINFRAME*/);

	HRESULT hResult = NULL;

	hResult = DllWndEngine::DllCoCreateObject(CLSID_IWndEngine, IID_IWndEngine, (void**)&mPtrWndEngine);
	if (hResult != S_OK)
	{
		AfxMessageBox(TEXT("Create CLSID_IWndEngine failed!\n"), MB_ICONERROR | MB_TOPMOST);
		ExitProcess(0);
	}
	hResult = DllProcEngine::DllCoCreateObject(CLSID_IProcEngine, IID_IProcEngine, (void**)&mPtrProcEngine);
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

	mPtrWndEngine->Release();
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

#include <Psapi.h>

bool CWndModuleDlg::UpdateWndModuleInfo(LPCTSTR lpTstr, HWND hFwnd, DWORD dwPid)
{

	this->UpdateWinProcessInfo(hFwnd, dwPid);

	if (m_hFwnd != hFwnd || dwPid)
	{
		m_hFwnd = hFwnd;
		GetWindowThreadProcessId(m_hFwnd, &m_dwRemoteId);
		ListModulesForProcess(m_dwRemoteId);
	}
	if (this->IsWindowVisible() == false) {
		this->ShowWindow(SW_SHOW);
	}
	//else ::SwitchToThisWindow(m_hWnd,TRUE);;

	return true;
}
void CWndModuleDlg::ListModulesForProcess(const DWORD &dwProcessId)
{
	int nModule = mPtrProcEngine->GetProcessModulesNumber(dwProcessId);

	if (nModule > 0) {

		LPMI lpmi = new MODULE_INFO[nModule];
		mPtrProcEngine->GetProcessModulesInfos(dwProcessId, lpmi, nModule);
		this->AnalyseModule(lpmi, nModule);
		delete[] lpmi;
		CString	szString;
		szString.Format(TEXT("窗口进程有%d个模块。"), nModule);
		mSbStatus.SetWindowText(szString);
	}
	else
	{
		mArInvalidDll.RemoveAll();
		mLvModule.DeleteAllItems();

		CString	szString;
		szString.Format(TEXT("获取进程模块失败：%s"), GetLastErrorInfo());
		mSbStatus.SetWindowText(szString);
	}
}
HWND CWndModuleDlg::GetWndHandle()
{
	return m_hWnd;
}
bool CWndModuleDlg::CreateWndModule(CWnd* pParentWnd)
{
	return CWndModuleDlg::Create(ATL_MAKEINTRESOURCE(CWndModuleDlg::IDD), pParentWnd) != FALSE;
}
bool CWndModuleDlg::SetCallback(IWndModuleCallback* pRWndModule)
{
	if (pRWndModule && pRWndModule->OnCheckCallback())
	{
		mPtrRWndModule = pRWndModule;
		return true;
	}
	return false;
}
bool CWndModuleDlg::SetIFileEngine(IFileEngine*pIFileEngine)
{
	msPtrIFileEngine = pIFileEngine;

	if (msPtrIFileEngine != NULL)
	{
		bool bIsEmpty = true;
		msPtrIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_MODULE_BOOL_EMPTY, &bIsEmpty);
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

			msPtrIFileEngine->AddVarValue(cTypeData, sizeof(cTypeData) / sizeof(TYPEDATA));

#ifdef	_DEBUG
			printf("CWndModuleDlg::IFileEngine::bIsEmpty == true\n");
			msPtrIFileEngine->OutPutDataInfo();
#endif
		}
	}

	return msPtrIFileEngine != NULL;
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

	DDX_Control(pDX, IDC_LIST_MODULEVIEW, mLvModule);
	DDX_Control(pDX, IDC_LIST_PROCVIEW, mLvProcess);
	DDX_Control(pDX, IDC_STATE, mSbStatus);
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
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_MODULEVIEW, &CWndModuleDlg::OnLvnColumnClickListModuleView)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MODULEVIEW, &CWndModuleDlg::OnNMDblclkListModuleView)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_MODULEVIEW, &CWndModuleDlg::OnNMCustomDrawListModuleView)
	ON_WM_SIZING()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_PROCVIEW, &CWndModuleDlg::OnLvnColumnClickListProcessView)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PROCVIEW, &CWndModuleDlg::OnNMDblclkListProcessView)
	ON_LBN_DBLCLK(IDC_LIST_PROCVIEW, &CWndModuleDlg::OnLbnDblclkListProcessView)
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
		{ TEXT("CPU"),50,LVCFMT_RIGHT,ProcCPUage },
		{ TEXT("线程数"),60,LVCFMT_RIGHT,ProcThreads },
		{ TEXT("用户名"),150,LVCFMT_RIGHT, ProcUserName },
		{ TEXT("内存使用"),100,LVCFMT_RIGHT,ProcWorkingSet },
		{ TEXT("创建时间"),100,LVCFMT_RIGHT,ProcCreateTime },
		{ TEXT("运行时间"),100,LVCFMT_RIGHT,ProcRunTime },
		{ TEXT("平台"),60,LVCFMT_CENTER, ProcPlatform },
		{ TEXT("公司"),150,LVCFMT_LEFT,ProcCompany },
		{ TEXT("描述"),280,LVCFMT_LEFT,ProcDescription },
		{ TEXT("路径"),550,LVCFMT_LEFT,ProcPath },
		//{ TEXT("虚拟内存大小"),150,LVCFMT_RIGHT,VirtualBytes },
	};

	for (int i = 0; i < FileALLColumn; i++)
		mLvModule.InsertColumn(clhc[i].dwColumn, clhc[i].lpName, clhc[i].nFormat, clhc[i].dwSize);

	for (int i = 0; i < ProcALLColumn; i++)
		mLvProcess.InsertColumn(ProcLh[i].dwColumn, ProcLh[i].lpName, ProcLh[i].nFormat, ProcLh[i].dwSize);

	m_ImageList16.Create(16, 16, ILC_COLOR24 | ILC_MASK, 1, 1);
	m_ImageList32.Create(32, 32, ILC_COLOR24 | ILC_MASK, 1, 1);

	mLvModule.SetImageList(&m_ImageList16, LVSIL_SMALL);
	mLvModule.SetImageList(&m_ImageList32, LVSIL_NORMAL);
	mLvModule.SetExtendedStyle(LVS_EX_FULLROWSELECT |/*LVS_EX_GRIDLINES|*/LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES | LVS_EX_HEADERINALLVIEWS);

	mLvProcess.SetImageList(&m_ImageList16, LVSIL_SMALL);
	mLvProcess.SetImageList(&m_ImageList32, LVSIL_NORMAL);
	mLvProcess.SetExtendedStyle(LVS_EX_FULLROWSELECT
		| LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES
		| LVS_EX_DOUBLEBUFFER | LVS_EX_SNAPTOGRID
		| LVS_EX_REGIONAL | LVS_EX_BORDERSELECT
		| LVS_EX_MULTIWORKAREAS | LVS_EX_INFOTIP
		| LVS_EX_COLUMNOVERFLOW | LVS_EX_COLUMNSNAPPOINTS
		| LVS_EX_HEADERINALLVIEWS);

	SetWindowText(TEXT("进程模块"));
	//::SetWindowLong(m_hWnd, GWL_STYLE, ::GetWindowLong(m_hWnd, GWL_STYLE) | WS_THICKFRAME);


	if (msPtrIFileEngine != NULL)
	{
		bool bIsMax = false;
		bool bIsIconc = false;
		bool bIsVisble = false;
		CRect wRect(0, 0, 0, 0);

		msPtrIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_MODULE_BOOL_ISMAX, &bIsMax);
		msPtrIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_MODULE_BOOL_ISICON, &bIsIconc);
		msPtrIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_MODULE_BOOL_VISBILE, &bIsVisble);
		msPtrIFileEngine->GetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_TREEPROP, &m_TreeProp);
		msPtrIFileEngine->GetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWFRAME, &m_dwViewFrame);
		msPtrIFileEngine->GetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWSTYLE, &m_dwViewStyle);
		msPtrIFileEngine->GetVarValue(IFileEngine::VT_STRUCT, FV_MODULE_STRUCT_WRECT, &wRect);

		if (bIsMax)
			this->ShowWindow(SW_MAXIMIZE);
		else if (bIsIconc)
			this->ShowWindow(SW_MINIMIZE);
		else if (wRect.Width() && wRect.Height())
			this->MoveWindow(wRect.left, wRect.top, wRect.Width(), wRect.Height());

		ShowWindow(bIsVisble ? SW_SHOW : SW_HIDE);
	}

	DWORD dwIds[] = { ID_VIEW_PROCESS,ID_VIEW_MODULE,ID_VIEW_PROCMOD };
	DWORD dwSty[] = { ViewProcess,Module,ProcMod };

	for (int i = 0; i < sizeof(dwIds) / sizeof(DWORD); i++) {
		if (m_dwViewFrame == dwSty[i]) {
			this->OnCommand(MAKEWPARAM(dwIds[i], 0), LPARAM(m_hWnd));
		}
	}

	this->OnCommand(MAKEWPARAM(m_dwViewStyle, 0), LPARAM(m_hWnd));

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
			mArInvalidDll.Add(CString(szDllName));
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
		this->ListModulesForProcess(m_dwRemoteId);
	}
	else if (nID == ID_CLOSE)
	{
		FORWARD_WM_SYSCOMMAND(m_hWnd, SC_CLOSE, 0, 0, ::SendMessage);
	}
	else if (nID == ID_VIEW_PROCESS)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_MODULE, ID_VIEW_PROCESS, nID, MF_CHECKED | MF_BYCOMMAND);

		m_dwViewFrame = ViewProcess;
		if (msPtrIFileEngine != NULL)
		{
			msPtrIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWFRAME, &m_dwViewFrame);
		}
		::ShowWindow(mLvProcess.m_hWnd, SW_SHOW);
		::ShowWindow(mLvModule.m_hWnd, SW_HIDE);

		this->UpdateWindowsFrame(m_ClientSize.cx, m_ClientSize.cy);
	}
	else if (nID == ID_VIEW_MODULE)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_MODULE, ID_VIEW_PROCESS, nID, MF_CHECKED | MF_BYCOMMAND);

		m_dwViewFrame = Module;
		if (msPtrIFileEngine != NULL)
		{
			msPtrIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWFRAME, &m_dwViewFrame);
		}
		::ShowWindow(mLvProcess.m_hWnd, SW_HIDE);
		::ShowWindow(mLvModule.m_hWnd, SW_SHOW);
		this->UpdateWindowsFrame(m_ClientSize.cx, m_ClientSize.cy);

	}
	else if (nID == ID_VIEW_PROCMOD)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_MODULE, ID_VIEW_PROCESS, nID, MF_CHECKED | MF_BYCOMMAND);

		m_dwViewFrame = ProcMod;
		if (msPtrIFileEngine != NULL)
		{
			msPtrIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWFRAME, &m_dwViewFrame);
		}
		::ShowWindow(mLvProcess.m_hWnd, SW_SHOW);
		::ShowWindow(mLvModule.m_hWnd, SW_SHOW);
		this->UpdateWindowsFrame(m_ClientSize.cx, m_ClientSize.cy);
	}
	else if (nID == ID_VIEW_ICON)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_ICON, ID_VIEW_LISTER, nID, MF_CHECKED | MF_BYCOMMAND);

		if (msPtrIFileEngine != NULL)
		{
			msPtrIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWSTYLE, &nID);
		}
		DWORD dwStyle = mLvModule.GetStyle() & ~(0x0003);

		::SetWindowLong(mLvModule.m_hWnd, GWL_STYLE, dwStyle | LVS_ICON);
	}
	else if (nID == ID_VIEW_SICON)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_ICON, ID_VIEW_LISTER, nID, MF_CHECKED | MF_BYCOMMAND);

		if (msPtrIFileEngine != NULL)
		{
			msPtrIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWSTYLE, &nID);
		}
		DWORD dwStyle = mLvModule.GetStyle() & ~(0x0003);

		::SetWindowLong(mLvModule.m_hWnd, GWL_STYLE, dwStyle | LVS_SMALLICON);
	}
	else if (nID == ID_VIEW_REPORT)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_ICON, ID_VIEW_LISTER, nID, MF_CHECKED | MF_BYCOMMAND);

		if (msPtrIFileEngine != NULL)
		{
			msPtrIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWSTYLE, &nID);
		}
		DWORD dwStyle = mLvModule.GetStyle() & ~(0x0003);

		::SetWindowLong(mLvModule.m_hWnd, GWL_STYLE, dwStyle | LVS_REPORT);
	}
	else if (nID == ID_VIEW_LISTER)
	{
		CMenu* pMenu = GetMenu()->GetSubMenu(2);
		pMenu->CheckMenuRadioItem(ID_VIEW_ICON, ID_VIEW_LISTER, nID, MF_CHECKED | MF_BYCOMMAND);

		if (msPtrIFileEngine != NULL)
		{
			msPtrIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_VIEWSTYLE, &nID);
		}
		DWORD dwStyle = mLvModule.GetStyle() & ~(0x0003);

		::SetWindowLong(mLvModule.m_hWnd, GWL_STYLE, dwStyle | LVS_LIST);
	}
	else if (nID == ID_MODULE_OPENPATH)
	{
		TCHAR	command[MAX_PATH], szPath[MAX_PATH];

		CMenu* pMenu = GetMenu()->GetSubMenu(1);

		if (pMenu->GetMenuState(ID_MODULE_UNLOAD, MF_BYCOMMAND) & MF_GRAYED) {
			mLvProcess.GetItemText(mLvProcess.GetSelectionMark(), ProcPath, szPath, 256);
		} else {
			mLvModule.GetItemText(mLvModule.GetSelectionMark(), FilePath, szPath, 256);
		}

		StringCchPrintf(command, MAX_PATH, TEXT("/select,%s"), szPath);

		ShellExecute(NULL, TEXT("open"), TEXT("Explorer.exe"), command, TEXT(""), SW_SHOW);


	}
	else if (nID == ID_MODULE_COPY_ROW)
	{
		int columns = 0;
		CString text;
		CListCtrl* pListView = nullptr;
		CMenu* pMenu = GetMenu()->GetSubMenu(1);
		if (pMenu->GetMenuState(ID_MODULE_UNLOAD, MF_BYCOMMAND) & MF_GRAYED) {
			columns = ProcALLColumn;
			pListView = &mLvProcess;
		}
		else {
			columns = FileALLColumn;
			pListView = &mLvModule;
		}
		int select = pListView->GetSelectionMark();

		for (int i = 0; i < columns; i++)
		{
			text += pListView->GetItemText(select, i);
			text += TEXT(" \t");
		}
		WriteClipboardText(text);
	}
	else if (nID == ID_MODULE_COPY_CULUMN)
	{
		CString text;
		CListCtrl* pListView = nullptr;
		CMenu* pMenu = GetMenu()->GetSubMenu(1);
		if (pMenu->GetMenuState(ID_MODULE_UNLOAD, MF_BYCOMMAND) & MF_GRAYED) {
			pListView = &mLvProcess;
		}
		else {
			pListView = &mLvModule;
		}
		int culumn = pListView->GetSelectedColumn();
		for (int i = 0; i < pListView->GetItemCount(); i++)
		{
			text += pListView->GetItemText(i, culumn);
			text += TEXT("\r\n");
		}
		WriteClipboardText(text);
	}
	else if (nID == ID_MODULE_COPY_CELL)
	{
		int columns = 0;
		CListCtrl* pListView = nullptr;
		CMenu* pMenu = GetMenu()->GetSubMenu(1);
		if (pMenu->GetMenuState(ID_MODULE_UNLOAD, MF_BYCOMMAND) & MF_GRAYED) {
			columns = ProcALLColumn;
			pListView = &mLvProcess;
		}
		else {
			columns = FileALLColumn;
			pListView = &mLvModule;
		}
		WriteClipboardText(pListView->GetItemText(pListView->GetSelectionMark(), pListView->GetSelectedColumn()));
	}
	else if (nID == ID_MODULE_COPY_TABLE)
	{
		int columns = 0;
		CString text;
		CListCtrl* pListView = nullptr;
		CMenu* pMenu = GetMenu()->GetSubMenu(1);
		if (pMenu->GetMenuState(ID_MODULE_UNLOAD, MF_BYCOMMAND) & MF_GRAYED) {
			columns = ProcALLColumn;
			pListView = &mLvProcess;
		}
		else {
			columns = FileALLColumn;
			pListView = &mLvModule;
		}
		for (int r = 0; r < pListView->GetItemCount(); r++)
		{
			for (int c = 0; c < columns; c++)
			{
				text += pListView->GetItemText(r, c);
				text += TEXT(" \t");
			}
			text += TEXT("\r\n");
		}
		WriteClipboardText(text);
	}
	else if (nID == ID_MODULE_INJECT)
	{
		TCHAR szLastFileName[MAX_PATH] = TEXT("");
		if (msPtrIFileEngine != nullptr) {
			msPtrIFileEngine->GetVarString(FV_MODULE_STRING_INJECT, szLastFileName, MAX_PATH);
		}
		CFileDialog dialog(true, TEXT("*.dll"), szLastFileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,TEXT("动态链接库(*.dll)|*.dll|所有文件 (*.*)|*.*||"),this,0,true);

		if (dialog.DoModal() == IDOK) {
			CString& szPathName = dialog.GetPathName();
			msPtrIFileEngine->PutVarString(FV_MODULE_STRING_INJECT, szPathName);

			CString& szPid = mLvProcess.GetItemText(mLvProcess.GetSelectionMark(), ProcessId);
			if (mPtrProcEngine->InjectDllToProcess(DWORD(_tcstol(szPid, NULL, 10)), LPCTSTR(szPathName))) {
				AfxMessageBox(TEXT("远程注入成功"));
			}
			else
			{
				AfxMessageBox(mPtrProcEngine->GetLastError());
			}
		}

		//explicit CFileDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		//	LPCTSTR lpszDefExt = NULL,
		//	LPCTSTR lpszFileName = NULL,
		//	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		//	LPCTSTR lpszFilter = NULL,
		//	CWnd* pParentWnd = NULL,
		//	DWORD dwSize = 0,
		//	BOOL bVistaStyle = TRUE);

	}
	else if (nID == ID_MODULE_UNLOAD)
	{
		TCHAR	szDllName[MAX_PATH] = TEXT("NULL");
		mLvModule.GetItemText(mLvModule.GetSelectionMark(), FileName, szDllName, MAX_PATH);

		if (lstrcmp(szDllName, TEXT("NULL")) == 0)
		{
			MessageBox(TEXT("读取项目为空"), TEXT("提示"), MB_ICONWARNING | MB_TOPMOST);
			return CWnd::OnCommand(wParam, lParam);
		}

		INT_PTR len = mArInvalidDll.GetSize();
		if (len != 0)
		{
			CString	szItem = szDllName;
			for (INT_PTR i = 0; i < len; i++)
			{
				if (mArInvalidDll.ElementAt(i) == szItem)
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
		CString szPid = mLvProcess.GetItemText(mLvProcess.GetSelectionMark(), ProcessId);
		if (szPid.IsEmpty() == false)
		{
			if (mPtrWndEngine->QueryProcess(DWORD(_tcstol(szPid, NULL, 10))) == false)
			{
				MessageBox(mPtrWndEngine->GetStageInfo(IWndEngine::LAST_ERROR), TEXT("提示"), MB_ICONWARNING | MB_TOPMOST);
				return CWnd::OnCommand(wParam, lParam);
			}
			if (MessageBox(TEXT("确定要结束进程吗？"), TEXT("提问"), MB_ICONQUESTION | MB_TOPMOST | MB_YESNO) != IDYES)
			{
				return CWnd::OnCommand(wParam, lParam);
			}

			if (mPtrWndEngine->SendFunction(IWndEngine::FUN_KILLPROCESS) == true)
			{
				MessageBox(TEXT("结束进程成功"), TEXT("提示"), MB_ICONWARNING | MB_TOPMOST);
				//FORWARD_WM_COMMAND(m_hWnd,ID_MODULE_REFASH,hCtl,code,::PostMessage);
			}
			else
			{
				MessageBox(mPtrWndEngine->GetStageInfo(IWndEngine::LAST_ERROR), TEXT("提示"), MB_ICONWARNING | MB_TOPMOST);
			}
		}
	}
	else if (nID == ID_MODULE_ATTRIB)
	{
		TCHAR	szPath[MAX_PATH];

		CMenu* pMenu = GetMenu()->GetSubMenu(1);

		if (pMenu->GetMenuState(ID_MODULE_UNLOAD, MF_BYCOMMAND) & MF_GRAYED)
		{
			mLvProcess.GetItemText(mLvProcess.GetSelectionMark(), ProcessId, szPath, 256);
			mPtrWndEngine->QueryProcess(DWORD(_tcstol(szPath, NULL, 10)));
			lstrcpy(szPath, mPtrWndEngine->GetStageInfo(IWndEngine::WND_PROCESSPATH));
		}
		else
		{
			POSITION pos = mLvModule.GetFirstSelectedItemPosition();
			mLvModule.GetItemText(mLvModule.GetNextSelectedItem(pos), FilePath, szPath, MAX_PATH);
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

		CString szPid = mLvProcess.GetItemText(mLvProcess.GetSelectionMark(), ProcessId);
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
					if (false == mPtrWndEngine->SetPriorityClass(DWORD(_tcstol(szPid, NULL, 10)), PCL[i])) {
						MessageBox(mPtrWndEngine->GetStageInfo(IWndEngine::LAST_ERROR), TEXT("提示"), MB_ICONWARNING | MB_TOPMOST);
					}
					break;
				}
			}
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}

void CWndModuleDlg::WriteClipboardText(CString &text)
{
	OpenClipboard();
	EmptyClipboard();
	size_t size = (text.GetLength() + 1) * sizeof(TCHAR);
	HGLOBAL hClipBuffer = GlobalAlloc(GMEM_MOVEABLE, size);
	CopyMemory(GlobalLock(hClipBuffer), LPCTSTR(text), size);
#ifdef UNICODE
	SetClipboardData(CF_UNICODETEXT, hClipBuffer);
#else
	SetClipboardData(CF_TEXT, hClipBuffer);
#endif // UNICODE
	GlobalUnlock(hClipBuffer);
	CloseClipboard();
}

void CWndModuleDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: 在此处添加消息处理程序代码
	if (pWnd->m_hWnd == mLvModule.m_hWnd)
	{
		POINT	pt = { point.x,point.y };
		::ScreenToClient(mLvModule.m_hWnd, &pt);

		int index = mLvModule.GetSelectionMark();

		CRect	rc;
		mLvModule.GetItemRect(index, &rc, 0);

		//HDC hdc = ::GetDC(pWnd->m_hWnd);
		//::FillRect(hdc,&rc,GetStockBrush(BLACK_BRUSH));
		//::ReleaseDC(pWnd->m_hWnd,hdc);

		if (PtInRect(&rc, pt))
		{
			for (int i = 1; i <= FileALLColumn; i++)
			{
				mLvModule.GetSubItemRect(index, i, LVIR_BOUNDS, rc);
				if (PtInRect(&rc, pt))
				{
					mLvModule.SetSelectedColumn(i);
					break;
				}
			}
			CMenu* pMenu = GetMenu()->GetSubMenu(1);
			pMenu->EnableMenuItem(ID_MODULE_UNLOAD, MF_ENABLED);
			pMenu->EnableMenuItem(ID_MODULE_INJECT, MF_GRAYED);
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
	else if (pWnd->m_hWnd == mLvProcess.m_hWnd)
	{
		CRect	rc;
		POINT	pt = { point.x,point.y };
		int index = mLvProcess.GetSelectionMark();

		::ScreenToClient(mLvProcess.m_hWnd, &pt);
		mLvProcess.GetItemRect(index, &rc, 0);


		if (PtInRect(&rc, pt))
		{
			for (int i = 1; i <= ProcALLColumn; i++)
			{
				mLvProcess.GetSubItemRect(index, i, LVIR_BOUNDS, rc);
				if (PtInRect(&rc, pt))
				{
					mLvProcess.SetSelectedColumn(i);
					break;
				}
			}
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
			pMenu->EnableMenuItem(ID_MODULE_INJECT, MF_ENABLED);
			pMenu->EnableMenuItem(ID_MODULE_UNLOAD, MF_GRAYED);

			CString szPid = mLvProcess.GetItemText(index, ProcessId);
			mPtrWndEngine->GetPriorityClass(DWORD(_tcstol(szPid, NULL, 10)), &pCl);

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

				//::MoveWindow(mLvProcess.m_hWnd, 0, 0, point.x, cy - 18, TRUE);
				//::MoveWindow(mLvModule.m_hWnd, point.x + 3, 0, cx - point.x - 3, cy - 18, TRUE);

				m_TreeProp = point.x;

				this->UpdateWindowsFrame(cx, cy);

				if (msPtrIFileEngine != NULL)
				{
					msPtrIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_MODULE_LONG_TREEPROP, &m_TreeProp);
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
	
	if (FALSE == ::IsWindow(mSbStatus.m_hWnd))
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
	if (bShow) {
		this->CenterWindow();
		this->SetTimer(TIME_UPDATE_PROC, 500, NULL);
	} else {
		this->KillTimer(TIME_UPDATE_PROC);
	}
}

//long CWndModuleDlg::AnalyseModule(LPCTSTR lpStr)
//{
//	mArInvalidDll.RemoveAll();
//	mLvModule.DeleteAllItems();
//
//	CString	cString = lpStr, szModPath, szModule;
//	CFileStatus cFileStatus;
//	CFileVersion cFileVersion;
//
//	TCHAR szFile[MAX_PATH] = TEXT("[NULL]"), szType[MAX_PATH] = TEXT("[NULL]");
//
//	int iCode = 0;
//	for (int i = 0, j = 0, n = 0, code; (i = cString.Find(TEXT("\r\n"), n)) != -1; n = i + 2)
//	{
//		if (i > n)
//		{
//			szModPath = cString.Mid(n, i - n);
//			_tsplitpath_s(szModPath, 0, 0, 0, 0, szFile, MAX_PATH, szType, MAX_PATH);
//			szModule.Format(TEXT("%s%s"), szFile, szType);
//
//			code = GetTypeIconCode(szType, szModPath);
//
//			iCode++;
//			cFileVersion.SetFilePath(szModPath);
//			mLvModule.InsertItem(j, NULL, code);
//			mLvModule.SetItemText(j, FileName, szModule);
//			mLvModule.SetItemText(j, FileType, szType);
//			mLvModule.SetItemText(j, FilePath, szModPath);
//			mLvModule.SetItemText(j, FileCompany, cFileVersion.GetCompanyName());
//			mLvModule.SetItemText(j, FileDescription, cFileVersion.GetFileDescription());
//
//			if (CFile::GetStatus(szModPath, cFileStatus)) {
//				TCHAR szSize[64];
//				auto size = cFileStatus.m_size;
//				if (size > (SPACE_SIZE_UNIT*SPACE_SIZE_UNIT))
//					StringCbPrintf(szSize, sizeof(szSize), TEXT("%.2fMB"), double(size) / (SPACE_SIZE_UNIT*SPACE_SIZE_UNIT));
//				else if (size > SPACE_SIZE_UNIT)
//					StringCbPrintf(szSize, sizeof(szSize), TEXT("%.2fKB"), double(size) / SPACE_SIZE_UNIT);
//				else
//					StringCbPrintf(szSize, sizeof(szSize), TEXT("%字节"), size);
//				mLvModule.SetItemText(j, FileSize, szSize);
//			} else {
//				mLvModule.SetItemText(j, FileSize, GetLastErrorInfo());
//			}
//		}
//	}
//
//	return iCode;
//}
long CWndModuleDlg::AnalyseModule(LPMI lpmi, UINT num)
{
	CString	szModule;
	CFileStatus cFileStatus;
	TCHAR szFile[MAX_PATH] = TEXT("[NULL]"), szType[MAX_PATH] = TEXT("[NULL]");

	mArInvalidDll.RemoveAll();
	mLvModule.DeleteAllItems();

	for (UINT i = 0; i < num; i++)
	{
		_tsplitpath_s(lpmi[i].strFilePath, 0, 0, 0, 0, szFile, MAX_PATH, szType, MAX_PATH);
		szModule.Format(TEXT("%s%s"), szFile, szType);
		mLvModule.InsertItem(i, NULL, GetTypeIconCode(szType, lpmi[i].strFilePath));
		mLvModule.SetItemText(i, FileName, szModule);
		mLvModule.SetItemText(i, FileType, szType);
		mLvModule.SetItemText(i, FilePath, lpmi[i].strFilePath);
		mLvModule.SetItemText(i, FileCompany, lpmi[i].strCompany);
		mLvModule.SetItemText(i, FileDescription, lpmi[i].strDescription);

		if (CFile::GetStatus(lpmi[i].strFilePath, cFileStatus)) {
			TCHAR szSize[64];
			auto size = cFileStatus.m_size;
			if (size > (SPACE_SIZE_UNIT*SPACE_SIZE_UNIT))
				StringCbPrintf(szSize, sizeof(szSize), TEXT("%.2fMB"), double(size) / (SPACE_SIZE_UNIT*SPACE_SIZE_UNIT));
			else if (size > SPACE_SIZE_UNIT)
				StringCbPrintf(szSize, sizeof(szSize), TEXT("%.2fKB"), double(size) / SPACE_SIZE_UNIT);
			else
				StringCbPrintf(szSize, sizeof(szSize), TEXT("%字节"), size);
			mLvModule.SetItemText(i, FileSize, szSize);
		}
		else {
			mLvModule.SetItemText(i, FileSize, GetLastErrorInfo());
		}
	}

	return num;
}
void CWndModuleDlg::UpdateWinProcessInfo(HWND hwnd, DWORD dwPid)
{
	UpdateProcessList();
	
	DWORD dwWndPid = dwPid;
	if (dwWndPid == 0) {
		::GetWindowThreadProcessId(hwnd, &dwWndPid);
	}
	for (int i = 0; i < mLvProcess.GetItemCount(); i++) {
		DWORD dwPid = DWORD(_tcstol(mLvProcess.GetItemText(i, ProcessId), NULL, 10));
		if (dwWndPid == dwPid) {
			int select = mLvProcess.GetSelectionMark();
			if (select >= 0) {
				mLvProcess.SetItemState(select, 0, LVIS_SELECTED | LVIS_FOCUSED);
			}
			mLvProcess.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			//mLvProcess.SetItemState(0,0xFFFFFFFF,LVIS_SELECTED); 
			mLvProcess.EnsureVisible(i, true);
			break;
		}
	}
}

void CWndModuleDlg::AddListItemForProcess(PCPI lpcProcess)
{
	mLvProcess.InsertItem(0, NULL, GetTypeIconCode(TEXT(".exe"), lpcProcess->strFilePath));
	PutListItemForProcess(0, lpcProcess);
}

void CWndModuleDlg::PutListItemForProcess(int index, PCPI lpcProcess)
{
	CString strFormat;

	mLvProcess.SetItemText(index, ProcName, lpcProcess->strProcessName);

	strFormat.Format(TEXT("%04d"), lpcProcess->dwProcessID);
	mLvProcess.SetItemText(index, ProcessId, strFormat);

	strFormat.Format(TEXT("%d"), lpcProcess->dwThreadsCount);
	mLvProcess.SetItemText(index, ProcThreads, strFormat);

	mLvProcess.SetItemText(index, ProcUserName, lpcProcess->strUserName);
	mLvProcess.SetItemText(index, ProcPath, lpcProcess->strFilePath);
	mLvProcess.SetItemText(index, ProcDescription, lpcProcess->strDescription);
	mLvProcess.SetItemText(index, ProcCompany, lpcProcess->strCompanyName);

	strFormat.Format(TEXT("%02d"), lpcProcess->ulCpuPage);
	mLvProcess.SetItemText(index, ProcCPUage, strFormat);
	mLvProcess.SetItemText(index, ProcPlatform, lpcProcess->blIsWin32 ? TEXT("x86") : TEXT("x64"));

	auto lpcMemory = &lpcProcess->miMemoryInfo;
	if (lpcMemory->dwWorkingSet > 0) {

		CTime cTime = lpcProcess->tiCpuTimeInfo.CreationTime;
		mLvProcess.SetItemText(index, ProcCreateTime, cTime.Format(TEXT("%H:%M:%S")));

		CTimeSpan tSpan = CTime::GetCurrentTime() - cTime;
		mLvProcess.SetItemText(index, ProcRunTime, tSpan.Format(TEXT("%H:%M:%S")));

		if (lpcMemory->dwWorkingSet > (SPACE_SIZE_UNIT*SPACE_SIZE_UNIT))
			strFormat.Format(TEXT("%.2fMB"), double(lpcMemory->dwWorkingSet) / (SPACE_SIZE_UNIT*SPACE_SIZE_UNIT));
		else if (lpcMemory->dwWorkingSet > SPACE_SIZE_UNIT)
			strFormat.Format(TEXT("%.2fKB"), double(lpcMemory->dwWorkingSet) / SPACE_SIZE_UNIT);
		else
			strFormat.Format(TEXT("%字节"), lpcMemory->dwWorkingSet);

		mLvProcess.SetItemText(index, ProcWorkingSet, strFormat);

		//if ((temp = lpcMemory->dwVirtualBytes / (1000 * 1000)) != 0)
		//	szString.Format(TEXT("%d,%03dK"), temp, (lpcMemory->dwVirtualBytes % (1000 * 1000)) / 1000);
		//else
		//	szString.Format(TEXT("%dK"), lpcMemory->dwVirtualBytes / 1000);

		//mLvProcess.SetItemText(index, VirtualBytes, szString);
	}
	else
	{
		mLvProcess.SetItemText(index, ProcCPUage, TEXT("00"));
		mLvProcess.SetItemText(index, ProcWorkingSet, TEXT("0K"));
	}


}

void CWndModuleDlg::UpdateWindowsFrame(int cx, int cy)
{
	if (mSbStatus.m_hWnd == nullptr) {
		return;
	}
	if (m_dwViewFrame == ProcMod)
	{
		if (!m_TreeProp) {
			CRect crect;
			mLvProcess.GetWindowRect(&crect);
			m_TreeProp = crect.Width();
			if (msPtrIFileEngine != NULL) {
				msPtrIFileEngine->PutVar(FV_MODULE_LONG_TREEPROP, m_TreeProp);
			}
		}
		int	iPos = int(m_TreeProp);
		::MoveWindow(mLvProcess.m_hWnd, 0, 1, iPos, cy - 23, TRUE);
		::MoveWindow(mLvModule.m_hWnd, iPos + 3, 1, cx - iPos - 3, cy - 23, TRUE);
	}
	else if (m_dwViewFrame == ViewProcess)
	{
		::MoveWindow(mLvProcess.m_hWnd, 3, 1, cx - 6, cy - 23, TRUE);
	}
	else if (m_dwViewFrame == Module)
	{
		::MoveWindow(mLvModule.m_hWnd, 3, 1, cx - 6, cy - 23, TRUE);
	}
	::MoveWindow(mSbStatus.m_hWnd, 0, cy - 19, cx, 19, TRUE);
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
void CWndModuleDlg::OnNMDblclkListModuleView(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码

	TCHAR	szPath[MAX_PATH];

	int index = mLvModule.GetSelectionMark();

	mLvModule.GetItemText(index, FilePath, szPath, 256);

	ShellExecute(NULL, TEXT("open"), szPath, TEXT(""), TEXT(""), SW_SHOW);

#ifdef _DEBUG
	_tprintf(TEXT("CWndModuleDlg::%s%s"), szPath, ::GetLastErrorInfo());
#endif

	*pResult = 0;
}

void CWndModuleDlg::OnNMCustomDrawListModuleView(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		//这里可以编写代码设置某一行的颜色
		INT_PTR len = mArInvalidDll.GetSize();
		if (len != 0)
		{
			int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);
			CString	szItem = mLvModule.GetItemText(int(nItem), FileName);
			for (INT_PTR i = 0; i < len; i++)
			{
				if (mArInvalidDll.ElementAt(i) == szItem)
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
	{	
		//这里可以设定具体某一行某一格的背景颜色
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

void CWndModuleDlg::OnLbnDblclkListProcessView()
{
	// TODO: 在此添加控件通知处理程序代码
	if (this->m_dwViewFrame != ProcMod)
	{
		FORWARD_WM_COMMAND(m_hWnd, ID_VIEW_PROCMOD, m_hWnd, 0, ::SendMessage);
	}
	CString& szText = mLvProcess.GetItemText(mLvProcess.GetSelectionMark(), ProcessId);
	//m_dwRemoteId = DWORD(_tcstol(szText,NULL,16));
	m_dwRemoteId = DWORD(_tcstol(szText, NULL, 10));
	this->ListModulesForProcess(m_dwRemoteId);
}

void CWndModuleDlg::OnNMDblclkListProcessView(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	this->OnLbnDblclkListProcessView();
	*pResult = 0;
}

void CWndModuleDlg::OnLvnColumnClickListProcessView(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	//这个for用来给表中的项添加上索引号
	for (int i = 0; i < mLvProcess.GetItemCount(); ++i) 
	{
		mLvProcess.SetItemData(i, i);
	}

	m_nfav = !m_nfav;

	DATA data = { m_nfav,m_dwProcItem = pNMLV->iSubItem,&mLvProcess };

	mLvProcess.SortItems(ProcCompare, (LPARAM)&data);

	*pResult = 0;
}

void CWndModuleDlg::OnLvnColumnClickListModuleView(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	for (int i = 0; i < mLvModule.GetItemCount(); ++i) //这个for用来给表中的项添加上索引号
	{
		mLvModule.SetItemData(i, i);
	}

	m_nfav = !m_nfav;

	DATA data = { m_nfav,pNMLV->iSubItem,&mLvModule };

	mLvModule.SortItems(ListCompare, (LPARAM)&data);

	*pResult = 0;
}

int	CALLBACK CWndModuleDlg::ProcCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	DATA* pData = (DATA*)lParamSort;

	int col = pData->iSubItem;//点击的列项传递给col，用来判断点击了第几列

	CString& strText1 = (pData->pListCtrl)->GetItemText(int(lParam1), col);
	CString& strText2 = (pData->pListCtrl)->GetItemText(int(lParam2), col);

	float cmp = 0;
	if (col == ProcThreads || col == ProcWorkingSet/* || col == VirtualBytes*/)
	{
		cmp = float(strText1.GetLength() - strText2.GetLength());
		if (cmp == 0)
			cmp = (float)lstrcmp(strText1, strText2);
	}
	else cmp = (float)lstrcmp(strText1, strText2);

	if (cmp > 0)return 1 - pData->nfav * 2;
	else if (cmp < 0)return pData->nfav * 2 - 1;
	else return int(cmp);

	return 0;
}

int	CALLBACK CWndModuleDlg::ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	DATA* pData = (DATA*)lParamSort;

	int col = pData->iSubItem;//点击的列项传递给col，用来判断点击了第几列
	CString& strText1 = (pData->pListCtrl)->GetItemText(int(lParam1), col);
	CString& strText2 = (pData->pListCtrl)->GetItemText(int(lParam2), col);

	float cmp = 0;
	if (col == FileSize)
	{
		cmp = float(strText1[strText1.GetLength() - 2] - strText2[strText2.GetLength() - 2]);
		
		if (cmp == 0)
			cmp = float(_tstof(strText1) - _tstof(strText2));
	}
	else
		cmp = (float)lstrcmp(strText1, strText2);

	if (cmp > 0)return 1 - pData->nfav * 2;
	else if (cmp < 0)return pData->nfav * 2 - 1;
	else return int(cmp);

	return 0;
}

void CWndModuleDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);

	// TODO: 在此处添加消息处理程序代码
	if (msPtrIFileEngine != NULL)
	{
		msPtrIFileEngine->SetVarValue(IFileEngine::VT_STRUCT, FV_MODULE_STRUCT_WRECT, pRect);
	}
}

void CWndModuleDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (TIME_UPDATE_PROC == nIDEvent && IsWindowVisible() && !IsIconic()) {
		UpdateProcessList();
		return ;
	}
	__super::OnTimer(nIDEvent);
}

void CWndModuleDlg::UpdateProcessList()
{
	List<LPCPI> ltPcProcess;
	long nProcess = mPtrProcEngine->UpdateProcessInfo();
	ltPcProcess.reserve(nProcess + 1);
	for (int i = 0; i < nProcess; i++) {
		ltPcProcess << mPtrProcEngine->GetProcessInfoByIndex(i);
	}
	ltPcProcess << nullptr;

	for (int i = 0; i < mLvProcess.GetItemCount(); i++) {
		LPCPI lpcProcess = nullptr;
		DWORD dwPid = DWORD(_tcstol(mLvProcess.GetItemText(i, ProcessId), NULL, 10));
		for (int n = 0; n < nProcess; n++) {
			if (ltPcProcess[n]->dwProcessID == dwPid) {
				lpcProcess = ltPcProcess[n];
				if (i < nProcess) {
					ltPcProcess[n] = ltPcProcess[i];
					ltPcProcess[i] = lpcProcess;
				}
				break;
			}
		}
		if (lpcProcess != nullptr) {
			this->PutListItemForProcess(i, lpcProcess);
		}
		else {
			mLvProcess.DeleteItem(i--);
		}
	}

	for (int len = mLvProcess.GetItemCount(); len < nProcess; len++) {
		this->AddListItemForProcess(ltPcProcess[len]);
	}

	//这个for用来给表中的项添加上索引号
	for (int i = 0; i < mLvProcess.GetItemCount(); ++i) {
		mLvProcess.SetItemData(i, i);
	}
	DATA data = { m_nfav,m_dwProcItem,&mLvProcess };
	mLvProcess.SortItems(ProcCompare, (LPARAM)&data);
}


LPVOID CWndModuleDlg::GetProcEngine()
{
	return LPVOID(mPtrProcEngine);
}
