// WndPreviewDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WindowsKiller3.0.h"
#include "WndPreviewDlg.h"
#include "afxdialogex.h"


// CWndPreviewDlg 对话框

struct DATA {
	BOOL		nfav;
	int			iSubItem;
	CListCtrl*	pListCtrl;
};
static	ULONG    g_LockNumber = 0;
static	ULONG    g_IWndPreviewNumber = 0;
static	HMODULE	 g_hModule = ::GetModuleHandle(NULL/*TEXT("WndPreview.dll")*/);

long					CWndPreviewDlg::m_nCode = 0;
CImageList				CWndPreviewDlg::m_ImageList;
CArray<ImageElem>		CWndPreviewDlg::m_ElemImage;
CArray<WndInfoElem>		CWndPreviewDlg::m_ElemModule;
CArray<WndInfoElem>		CWndPreviewDlg::m_ElemPeFile;

#define	FV_PREVIEW_BOOL_EMPTY		TEXT("PreView_bIsEmpty")
#define	FV_PREVIEW_BOOL_ISMAX		TEXT("PreView_bIsMax")
#define	FV_PREVIEW_BOOL_ISICON		TEXT("PreView_bIsIconc")
#define	FV_PREVIEW_BOOL_VISBILE		TEXT("PreView_bIsVisble")
#define	FV_PREVIEW_LONG_TREEPROP	TEXT("PreView_lTreeProp")
#define	FV_PREVIEW_STRUCT_WRECT		TEXT("PreView_strwRect")

IFileEngine*	CWndPreviewDlg::m_pIFileEngine = NULL;


IMPLEMENT_DYNAMIC(CWndPreviewDlg, CDialog)

CWndPreviewDlg::CWndPreviewDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_CHILDVIEW, pParent)
{
	//////////////////////////////////////////////////////////////////////////////
	//	Unknown
	m_Ref = 0;
	g_IWndPreviewNumber++;
	//////////////////////////////////////////////////////////////////////////////
	//	IWndPreview

	m_Filter = 0;
	m_TreeProp = 0;
	m_hFwnd = NULL;
	m_pRWndPreview = NULL;

	m_SetSize = false;

	m_hFwnd = NULL;
	m_hIcon = AfxGetApp()->LoadIcon(128/*IDR_MAINFRAME*/);


	HRESULT hResult = NULL;

	hResult = DllWndEngine::DllCoCreateObject(CLSID_IWndEngine, IID_IWndEngine, (void**)&mPtrWndEngine);
	if (hResult != S_OK)
	{
		AfxMessageBox(TEXT("CWndPreviewDlg::Create CLSID_IWndEngine failed!\n"), MB_ICONERROR | MB_TOPMOST);
		ExitProcess(0);
	}
	hResult = DllWndSigned::DllCoCreateObject(CLSID_IWndSigned, IID_IWndSigned, (void**)&m_pIWndSigned);
	if (hResult != S_OK)
	{
		AfxMessageBox(TEXT("Create CLSID_IWndSigned failed!\n"), MB_ICONERROR | MB_TOPMOST);
		ExitProcess(0);
	}
}

CWndPreviewDlg::~CWndPreviewDlg()
{
	mPtrWndEngine->Release();
	m_pIWndSigned->Release();
}

HRESULT  CWndPreviewDlg::QueryInterface(const IID& iid, void **ppv)
{
	if (iid == IID_IUnknown)
	{
		*ppv = (IUnknown *)this;
		((IUnknown *)(*ppv))->AddRef();
	}
	else if (iid == IID_IWndPreview)
	{
		*ppv = (IWndPreview *)this;
		((IWndPreview *)(*ppv))->AddRef();
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG	 CWndPreviewDlg::AddRef()
{
	m_Ref++;
	return  (ULONG)m_Ref;
}

ULONG	 CWndPreviewDlg::Release()
{
	m_Ref--;
	if (m_Ref == 0) {
		g_IWndPreviewNumber--;
		delete this;
		return 0;
	}
	return  (ULONG)m_Ref;
}
HWND CWndPreviewDlg::GetWndHandle()
{
	return m_hWnd;
}
bool CWndPreviewDlg::HideChildWndView()
{
	this->ShowWindow(SW_MINIMIZE);
	return true;
}
bool CWndPreviewDlg::ShowChildWndView()
{
	this->ShowWindow(SW_SHOWNORMAL);
	return true;
}
bool CWndPreviewDlg::CreateChildWndView(CWnd* pParentWnd)
{
	return CWndPreviewDlg::Create(ATL_MAKEINTRESOURCE(CWndPreviewDlg::IDD), pParentWnd) != FALSE;
}
bool CWndPreviewDlg::UpdateChildWndsInfo(HWND hWnd)
{
	mLvChildWnd.DeleteAllItems();
	mTvWindows.DeleteAllItems();

	if (EnumWindowChlid(::GetDesktopWindow(), hWnd) == false)
	{
		mTvWindows.SelectItem(mTvWindows.GetRootItem());
		mTvWindows.Expand(mTvWindows.GetRootItem(), TVE_EXPAND);
	}
	return true;
}
bool CWndPreviewDlg::SetRWndPreview(RWndPreview* pRWndPreview)
{
	m_pRWndPreview = pRWndPreview;
	if (pRWndPreview)
	{
		return pRWndPreview->OnCheckRWndPreview();
	}
	return false;
}
bool CWndPreviewDlg::DelRWndPreview(RWndPreview* pRWndPreview)
{
	if (m_pRWndPreview == pRWndPreview)
	{
		m_pRWndPreview = NULL;
		return true;
	}
	return false;
}
bool CWndPreviewDlg::SetIFileEngine(IFileEngine*pIFileEngine)
{
	m_pIFileEngine = pIFileEngine;

	if (m_pIFileEngine != NULL)
	{
		bool bIsEmpty = true;
		m_pIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_PREVIEW_BOOL_EMPTY, &bIsEmpty);
		if (bIsEmpty == true)
		{
			CRect		cRect(0, 0, 0, 0);
			TYPEDATA	cTypeData[] =
			{
			{ IFileEngine::VT_BOOL,FV_PREVIEW_BOOL_EMPTY,LPCVOID(false),TD_VALUE },
			{ IFileEngine::VT_BOOL,FV_PREVIEW_BOOL_ISMAX,LPCVOID(false),TD_VALUE },
			{ IFileEngine::VT_BOOL,FV_PREVIEW_BOOL_ISICON,LPCVOID(false),TD_VALUE },
			{ IFileEngine::VT_BOOL,FV_PREVIEW_BOOL_VISBILE,LPCVOID(false),TD_VALUE },
			{ IFileEngine::VT_LONG,FV_PREVIEW_LONG_TREEPROP,LPCVOID(0),TD_VALUE },
			{ IFileEngine::VT_STRUCT,FV_PREVIEW_STRUCT_WRECT,&cRect,sizeof(CRect) },
			};
			m_pIFileEngine->AddVarValue(cTypeData, sizeof(cTypeData) / sizeof(TYPEDATA));


#ifdef	_DEBUG
			printf("CWndPreviewDlg::IFileEngine::bIsEmpty == true\n");
			m_pIFileEngine->OutPutDataInfo();
#endif

		}
	}

	return m_pIFileEngine != NULL;
}
BOOL CWndPreviewDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	//ASSERT(pParentWnd != NULL);
	ASSERT(IS_INTRESOURCE(lpszTemplateName) || AfxIsValidString(lpszTemplateName));

	m_lpszTemplateName = lpszTemplateName;  // used for help
	if (IS_INTRESOURCE(m_lpszTemplateName) && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD_PTR)m_lpszTemplateName);

	HINSTANCE hInst = ::GetModuleHandle(NULL/*TEXT("WndPreview.dll")*/);
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);

	BOOL bResult = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);
	UnlockResource(hTemplate);
	FreeResource(hTemplate);

	return bResult;
}
void CWndPreviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CHILDVIEW, mLvChildWnd);
	DDX_Control(pDX, IDC_TREE_VIEW, mTvWindows);
	DDX_Control(pDX, IDC_STATE, mSbStatus);
}

BEGIN_MESSAGE_MAP(CWndPreviewDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZING()
	ON_BN_CLICKED(IDCANCEL, &CWndPreviewDlg::OnBnClickedButtonOkCancel)
	ON_BN_CLICKED(IDOK, &CWndPreviewDlg::OnBnClickedButtonOkCancel)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_CHILDVIEW, &CWndPreviewDlg::OnLvnColumnclickListChildview)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_VIEW, &CWndPreviewDlg::OnTvnSelchangedTreeView)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CHILDVIEW, &CWndPreviewDlg::OnNMDblclkListChildview)
END_MESSAGE_MAP()


// CWndPreviewDlg 消息处理程序


BOOL CWndPreviewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

									// TODO: 在此添加额外的初始化代码
	if (m_pIFileEngine != NULL)
	{
		bool bIsMax = false;
		bool bIsIconc = false;
		bool bIsVisble = false;

		CRect wRect(0, 0, 0, 0);

		m_pIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_PREVIEW_BOOL_ISMAX, &bIsMax);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_PREVIEW_BOOL_ISICON, &bIsIconc);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_BOOL, FV_PREVIEW_BOOL_VISBILE, &bIsVisble);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_LONG, FV_PREVIEW_LONG_TREEPROP, &m_TreeProp);
		m_pIFileEngine->GetVarValue(IFileEngine::VT_STRUCT, FV_PREVIEW_STRUCT_WRECT, &wRect);

		if (bIsMax)
			this->ShowWindow(SW_MAXIMIZE);
		else if (bIsIconc)
			this->ShowWindow(SW_MINIMIZE);
		else if (wRect.Width() && wRect.Height())
			this->MoveWindow(wRect.left, wRect.top, wRect.Width(), wRect.Height());

		ShowWindow(bIsVisble ? SW_SHOW : SW_HIDE);
	}

	::SetWindowLong(m_hWnd, GWL_STYLE, GetStyle() | WS_THICKFRAME);

	CRect	rtTree, rtClient;

	::GetClientRect(m_hWnd, &rtClient);
	::GetWindowRect(mTvWindows.m_hWnd, &rtTree);

	if (m_TreeProp == 0)
	{
		m_TreeProp = rtTree.Width();
	}

	CRect	cRect;
	GetClientRect(&cRect);
	int cx = cRect.Width(), cy = cRect.Height();
	this->UpdateWindowsFrame(cx, cy);

	mLvChildWnd.SetExtendedStyle(LVS_EX_FULLROWSELECT |/*LVS_EX_GRIDLINES|*/LVS_EX_SUBITEMIMAGES);

	CString strHeader[] = { TEXT("窗口句柄"),TEXT("窗口标题"), TEXT("窗口类名"), TEXT("进程ID"), TEXT("进程名"), TEXT("模块名"), TEXT("进程路径"), TEXT("模块路径") };
	int nLong[] = { 150, 200, 150,100,150,150,500,500 };
	for (int nCol = 0; nCol<sizeof(strHeader) / sizeof(CString); nCol++)
		mLvChildWnd.InsertColumn(nCol, strHeader[nCol], LVCFMT_LEFT, nLong[nCol]);


	if (m_nCode == 0)
	{
		m_nCode = 1;
		m_ImageList.Create(16, 16, ILC_COLOR24 | ILC_MASK, 2, 1/*16,16,ILC_COLOR24|ILC_MASK,1,1*/);

		SHFILEINFO   sfi;
		SHGetFileInfo(TEXT(".exe"), 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
		m_ImageList.Add(sfi.hIcon);
	}

	mTvWindows.SetImageList(&m_ImageList, TVSIL_NORMAL);
	mLvChildWnd.SetImageList(&m_ImageList, LVSIL_SMALL);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
long CWndPreviewDlg::AddItemSameAsProc(HTREEITEM hTreeItem, DWORD dwPid)
{
	HWND		hWnd = NULL;
	DWORD		dwGetPid = 0;
	HTREEITEM	hFTreeItem = NULL;
	int iCount = 0, nImage = 0, nSelectedImage = 0;

	while (hTreeItem)
	{
#ifdef _WIN64
		hWnd = HWND(_tcstoll(mTvWindows.GetItemText(hTreeItem), NULL, 16));
#else
		hWnd = HWND(_tcstol(mTvWindows.GetItemText(hTreeItem), NULL, 16));
#endif // _WIN64
		::GetWindowThreadProcessId(hWnd, &dwGetPid);

		if (dwGetPid == dwPid)
		{
			mTvWindows.GetItemImage(hTreeItem, nImage, nSelectedImage);
			AddWndToListCtrl(hWnd, 0, nImage, mTvWindows.GetItemText(hTreeItem));
			iCount++;
		}

		iCount += AddItemSameAsProc(mTvWindows.GetChildItem(hTreeItem), dwPid);

		hTreeItem = mTvWindows.GetNextSiblingItem(hTreeItem);
	}
	return iCount;
}
HTREEITEM CWndPreviewDlg::FindItemFormTree(HTREEITEM hTreeItem, HWND hDestWnd)
{
	HTREEITEM hFTreeItem = NULL;
	while (hTreeItem)
	{
#ifdef _WIN64
		HWND hWnd = HWND(_tcstoll(mTvWindows.GetItemText(hTreeItem), NULL, 16));
#else
		HWND hWnd = HWND(_tcstol(mTvWindows.GetItemText(hTreeItem), NULL, 16));
#endif // _WIN64
		if (hWnd == hDestWnd) {
			return hTreeItem;
		}

		hFTreeItem = FindItemFormTree(mTvWindows.GetChildItem(hTreeItem), hDestWnd);
		if (hFTreeItem != NULL)
		{
			return hFTreeItem;
		}

		hTreeItem = mTvWindows.GetNextSiblingItem(hTreeItem);
	}

	return NULL;
}
WndInfoElem* CWndPreviewDlg::AddExeFile(HWND hWnd, DWORD dwPid)
{
	mPtrWndEngine->QueryProcess(dwPid);

	m_ElemPeFile.Add(WndInfoElem(mPtrWndEngine->GetStageInfo(IWndEngine::WND_PROCESSPATH), dwPid));

	return &m_ElemPeFile.ElementAt(m_ElemPeFile.GetSize() - 1);
}
WndInfoElem* CWndPreviewDlg::GetExeFile(DWORD dwPid)
{
	WndInfoElem *pElem;
	for (INT_PTR i = 0, len = m_ElemPeFile.GetSize(); i < len; i++)
	{
		pElem = &m_ElemPeFile.ElementAt(i);

		if (pElem->dwPid == dwPid)
		{
			return pElem;
		}
	}
	return NULL;
}
WndInfoElem* CWndPreviewDlg::GetExeFile(HWND hWnd, LPCTSTR lphWnd)
{
	DWORD dwPid = 0;
	::GetWindowThreadProcessId(hWnd, &dwPid);

	WndInfoElem* pWndInfoElem = GetExeFile(dwPid);

	if (pWndInfoElem == NULL)
	{
		pWndInfoElem = AddExeFile(hWnd, dwPid);
	}

	return pWndInfoElem;
}
WndInfoElem* CWndPreviewDlg::AddtModuleFile(HWND hWnd, HMODULE hModule, DWORD dwPid)
{
	mPtrWndEngine->QueryWindow(hWnd);

	m_ElemPeFile.Add(WndInfoElem(mPtrWndEngine->GetStageInfo(IWndEngine::WND_MODULEPATH), hModule, dwPid));

	return &m_ElemPeFile.ElementAt(m_ElemPeFile.GetSize() - 1);
}
WndInfoElem* CWndPreviewDlg::GetModuleFile(HMODULE hModule, DWORD dwPid)
{
	WndInfoElem *pElem;
	for (INT_PTR i = 0, len = m_ElemPeFile.GetSize(); i < len; i++)
	{
		pElem = &m_ElemPeFile.ElementAt(i);

		if (pElem->hModule == hModule && pElem->dwPid == dwPid)
		{
			return pElem;
		}
	}
	return NULL;
}
WndInfoElem* CWndPreviewDlg::GetModuleFile(HWND hWnd, LPCTSTR lphWnd)
{
	DWORD	dwPid;
	HMODULE hModule = (HMODULE)::GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

	GetWindowThreadProcessId(hWnd, &dwPid);
	WndInfoElem* pWndInfoElem = GetModuleFile(hModule, dwPid);

	if (pWndInfoElem == NULL)
	{
		pWndInfoElem = AddtModuleFile(hWnd, hModule, dwPid);
	}

	return pWndInfoElem;
}
DWORD CWndPreviewDlg::GetTypeIconCode(HWND hWnd, LPCTSTR lpStr, DWORD dwPid)
{
	ImageElem *pElem, *pPidElem = NULL/*,*pWndElem = NULL*/;
	for (int i = 0; i < m_ElemImage.GetSize(); i++)
	{
		pElem = &m_ElemImage.ElementAt(i);

		if ((dwPid && dwPid == pElem->dwPid))
			pPidElem = pElem;
		if (pElem->szType.CompareNoCase(lpStr) == 0)
			return pElem->dwCode;
		/*if((dwPid && dwPid == pElem->dwPid)
		|| pElem->szType.CompareNoCase(lpStr) == 0)
		{
		return pElem->dwCode;
		}*/
	}
	if (pPidElem != NULL)
	{
		DWORD	dwCode = AddHandIconCode(hWnd, lpStr);
		if (dwCode == -1)
			return pPidElem->dwCode;
		else
			return dwCode;
	}
	return -1;
}
DWORD CWndPreviewDlg::GetTypeIconCode(HWND hWnd, LPCTSTR lphWnd)
{
	DWORD dwPid = 0;
	::GetWindowThreadProcessId(hWnd, &dwPid);

	DWORD dwCode = GetTypeIconCode(hWnd, lphWnd, dwPid);

	if (dwCode == -1)
	{
		dwCode = AddTypeIconCode(hWnd, lphWnd, dwPid);
	}
	return dwCode;

}
DWORD CWndPreviewDlg::AddTypeIconCode(HWND hWnd, LPCTSTR lphWnd, DWORD dwPid)
{
	DWORD	dwCode = AddHandIconCode(hWnd, lphWnd);
	if (dwCode == -1)
	{
		//mPtrWndEngine->QueryWindow(hWnd);
		dwCode = AddFileIconCode(GetExeFile(hWnd, lphWnd)->szPath, dwPid);
	}
	return dwCode;
}
DWORD CWndPreviewDlg::AddHandIconCode(HWND hWnd, LPCTSTR lphWnd)
{
	HICON	hIcon = NULL;

	SendMessageTimeout(hWnd, WM_GETICON, ICON_SMALL, 0, SMTO_NORMAL, 100, PDWORD_PTR(&hIcon));

	if (hIcon != NULL)
	{
		m_ImageList.Add(hIcon);
		m_ElemImage.Add(ImageElem(lphWnd, m_nCode));

		return m_nCode++;
	}
	return -1;
}
DWORD CWndPreviewDlg::AddFileIconCode(LPCTSTR lpFileName, DWORD dwPid)
{
	HICON	hIcon = ExtractIcon(g_hModule, lpFileName, 0);
	if (hIcon != NULL)
	{
		m_ImageList.Add(hIcon);
		m_ElemImage.Add(ImageElem(TEXT(""), m_nCode, dwPid));

		return m_nCode++;
	}
	return -1;
}

void CWndPreviewDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}
BOOL CWndPreviewDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int	 nID = LOWORD(wParam);
	HWND hCtl = HWND(lParam);
	UINT code = UINT(HIWORD(wParam));

	CString	szItemText;
	TCHAR	szhWnd[MAX_PATH] = TEXT("");
	int index = mLvChildWnd.GetSelectionMark();
	if (index != -1)
	{
		mLvChildWnd.GetItemText(index, 0, szhWnd, 256);
		szItemText = szhWnd;
	}
	else
	{
		HTREEITEM sht = mTvWindows.GetSelectedItem();

		if (sht != NULL)
			szItemText = mTvWindows.GetItemText(sht);
	}

	if (nID == ID_ABOUT)
	{
		CDialog	dlgAbout(100/*IDD_ABOUTBOX*/, this);
		dlgAbout.DoModal();
		return 0;
	}

	if (szItemText.IsEmpty() == false)
	{
#ifdef _WIN64
		HWND hWnd = HWND(_tcstoll(szItemText, NULL, 16));
#else
		HWND hWnd = HWND(_tcstol(szItemText, NULL, 16));
#endif // _WIN64
		DWORD	dwPid, dwTid = GetWindowThreadProcessId(hWnd, &dwPid);

		if (::IsWindow(hWnd) == FALSE && (ID_MENU_SIGN == nID || ID_MENU_SHOWTOP == nID || ID_MENU_HIDE == nID))
		{
			MessageBox(TEXT("窗口无效，请刷新。。"), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
			return CDialog::OnCommand(wParam, lParam);
		}

		if (code == EN_CHANGE)
		{

		}
		else if (ID_MENU_SIGN == nID)
		{
			CRect	cRect, dRect;
			::GetWindowRect(hWnd, &cRect);
			::GetWindowRect(::GetDesktopWindow(), &dRect);

			if (::IsIconic(hWnd))
				MessageBox(TEXT("窗口已经最小化。"), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
			else if (cRect.left > dRect.right || cRect.right < dRect.left || cRect.top > dRect.bottom || cRect.bottom < dRect.top)
				MessageBox(TEXT("窗口不在可视范围内。"), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
			else
				m_pIWndSigned->SignRect(&cRect, 2400);

		}
		else if (ID_MENU_SHOWTOP == nID)
		{
			if (dwPid == ::GetCurrentProcessId())
			{
				CWnd::MessageBox(TEXT("不能对本进程窗口操作。"), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
			}
			else if (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD)
			{
				CWnd::MessageBox(TEXT("只对顶层窗口有效。"), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
			}
			else
			{
				::ShowWindow(hWnd, SW_SHOW);
				::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			}
		}
		else if (ID_MENU_HIDE == nID)
		{
			if (dwPid == ::GetCurrentProcessId())
			{
				CWnd::MessageBox(TEXT("不能对本进程窗口操作。"), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
			}
			else if (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD)
			{
				CWnd::MessageBox(TEXT("只对顶层窗口有效。"), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
			}
			else
			{
				::ShowWindow(hWnd, SW_HIDE);
			}
		}
		else if (ID_MENU_INFO == nID)
		{
			if (m_pRWndPreview != NULL)
			{
				if (m_pRWndPreview->OnQueryWindow(hWnd) == false)
				{
					CWnd::MessageBox(m_pRWndPreview->OnGetLastError(), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
				}
			}
		}
		else if (ID_MENU_PROCINFO == nID)
		{
			if (m_pRWndPreview != NULL)
			{
				if (m_pRWndPreview->OnQueryProcess(dwPid) == false)
				{
					CWnd::MessageBox(m_pRWndPreview->OnGetLastError(), TEXT("提示"), MB_TOPMOST | MB_ICONWARNING);
				}
			}
		}
		else if (ID_MENU_RELATE == nID)
		{
			HTREEITEM hFTreeItem = FindItemFormTree(mTvWindows.GetRootItem(), hWnd);
			if (hFTreeItem != NULL)
			{
				if (mTvWindows.GetSelectedItem() != hFTreeItem)
				{
					m_Filter++;
					mTvWindows.SelectItem(hFTreeItem);
					mTvWindows.Expand(hFTreeItem, TVE_EXPAND);
				}

				CRect cRect;
				mTvWindows.GetItemRect(hFTreeItem, &cRect, FALSE);

				cRect.right -= cRect.left;
				cRect.bottom -= cRect.top;
				mTvWindows.ClientToScreen(LPPOINT(&cRect));
				cRect.right += cRect.left;
				cRect.bottom += cRect.top;

				m_pIWndSigned->SignRect(&cRect, 1000);
			}
		}
		else if (ID_MENU_SPROC == nID)
		{
			CString	szStr = mLvChildWnd.GetItemText(index, 3);

			mLvChildWnd.DeleteAllItems();

			long lConut = AddItemSameAsProc(mTvWindows.GetRootItem(), DWORD(_tcstol(szStr, NULL, 16)));

			szStr.Format(TEXT("进程%s一共有%d个窗口。"), LPCTSTR(mLvChildWnd.GetItemText(index, 4)), lConut);
			mSbStatus.SetWindowText(szStr);
		}
		else if (ID_CHILDWND_REFASH == nID)
		{
			TCHAR szClass[256], szText[256];

			m_nCode = 1;
			m_ImageList.DeleteImageList();
			m_ImageList.Create(16, 16, ILC_COLOR24 | ILC_MASK, 2, 1);

			SHFILEINFO   sfi;
			SHGetFileInfo(TEXT(".exe"), 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
			m_ImageList.Add(sfi.hIcon);

			mTvWindows.SetImageList(&m_ImageList, TVSIL_NORMAL);
			mLvChildWnd.SetImageList(&m_ImageList, LVSIL_SMALL);

			m_ElemImage.RemoveAll();
			m_ElemModule.RemoveAll();
			m_ElemPeFile.RemoveAll();


			m_Filter++;

#ifdef _WIN64
			hWnd = HWND(_tcstoll(mTvWindows.GetItemText(mTvWindows.GetSelectedItem()), NULL, 16));
#else
			hWnd = HWND(_tcstol(mTvWindows.GetItemText(mTvWindows.GetSelectedItem()), NULL, 16));
#endif // _WIN64

			mTvWindows.DeleteAllItems();

			if (EnumWindowChlid(::GetDesktopWindow(), hWnd) == false)
			{
				mTvWindows.SelectItem(mTvWindows.GetRootItem());
				mTvWindows.Expand(mTvWindows.GetRootItem(), TVE_EXPAND);
			}

			WndInfoElem* pWndInfoElem = NULL;
			for (int i = 0, len = mLvChildWnd.GetItemCount(); i < len; i++)
			{
#ifdef _WIN64
				hWnd = HWND(_tcstoll(mLvChildWnd.GetItemText(i, 0), NULL, 16));
#else
				hWnd = HWND(_tcstol(mLvChildWnd.GetItemText(i, 0), NULL, 16));
#endif // _WIN64
				if (::IsWindow(hWnd) == TRUE)
				{
					::GetWindowText(hWnd, szText, 256);
					::GetClassName(hWnd, szClass, 256);

					szItemText.Format(TEXT("%#08x|%s|%s"), hWnd, szClass, szText);

					pWndInfoElem = this->GetExeFile(hWnd, szItemText);
					if (pWndInfoElem != NULL)
					{
						mLvChildWnd.SetItemText(i, 4, pWndInfoElem->szFile);
						mLvChildWnd.SetItemText(i, 6, pWndInfoElem->szPath);
					}
					else
					{
						mLvChildWnd.SetItemText(i, 4, TEXT(""));
						mLvChildWnd.SetItemText(i, 6, TEXT(""));
					}
					pWndInfoElem = this->GetModuleFile(hWnd, szItemText);
					if (pWndInfoElem != NULL)
					{
						mLvChildWnd.SetItemText(i, 5, pWndInfoElem->szFile);
						mLvChildWnd.SetItemText(i, 7, pWndInfoElem->szPath);
					}
					else
					{
						mLvChildWnd.SetItemText(i, 5, TEXT(""));
						mLvChildWnd.SetItemText(i, 7, TEXT(""));
					}
				}
				else
				{
					mLvChildWnd.SetItemText(i, 0, TEXT("窗口无效"));
					mLvChildWnd.SetItemText(i, 1, TEXT(""));
					mLvChildWnd.SetItemText(i, 2, TEXT(""));
					mLvChildWnd.SetItemText(i, 3, TEXT(""));
					mLvChildWnd.SetItemText(i, 4, TEXT(""));
					mLvChildWnd.SetItemText(i, 5, TEXT(""));
					mLvChildWnd.SetItemText(i, 6, TEXT(""));
					mLvChildWnd.SetItemText(i, 7, TEXT(""));
					mLvChildWnd.SetItemState(i, 0, LVIF_IMAGE);
					//mLvModule.SetItem(i,0,0,"窗口无效",0,0,0,0);
				}

			}

			if (index != -1)
			{
				mLvChildWnd.SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				//mLvModule.SetItemState(index,0xFFFFFFFF,LVIS_SELECTED); 
			}
		}
	}
	else
	{
		MessageBox(TEXT("请先选择窗口。"));
	}

	return CDialog::OnCommand(wParam, lParam);
}
// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWndPreviewDlg::OnPaint()
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
HCURSOR CWndPreviewDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CWndPreviewDlg::OnClose()
{
	CDialog::OnClose();
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	this->ShowWindow(SW_HIDE);
	//this->GetParent()->SetActiveWindow();
}
void CWndPreviewDlg::UpdateWindowsFrame(int cx, int cy)
{
	int	iPos = int(m_TreeProp);

	::MoveWindow(mTvWindows.m_hWnd, 0, 1, iPos, cy - 23, TRUE);
	::MoveWindow(mLvChildWnd.m_hWnd, iPos + 3, 1, cx - iPos - 3, cy - 23, TRUE);
	::MoveWindow(mSbStatus.m_hWnd, 0, cy - 19, cx, 19, TRUE);
}
void CWndPreviewDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: 在此处添加消息处理程序代码
	m_ClientSize.cx = cx;
	m_ClientSize.cy = cy;

	if (FALSE == ::IsWindow(mSbStatus.m_hWnd))
	{
		m_ClientSize.cy -= 3;
	}

	this->UpdateWindowsFrame(cx, cy);
}
void CWndPreviewDlg::OnBnClickedButtonOkCancel()
{
	this->ShowWindow(SW_HIDE);
	//this->GetParent()->SetActiveWindow();
}
void CWndPreviewDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	// TODO: 在此处添加消息处理程序代码
	if (bShow)this->CenterWindow();
}

bool CWndPreviewDlg::EnumWindowChlid(HWND hPWnd, HWND hWnd, HTREEITEM faItem, int state)
{
	bool	bFind = false;
	HWND	hCWnd = hPWnd;

	if (::IsWindow(hCWnd) != FALSE)
	{
		TCHAR szText[MAX_PATH], szTitle[MAX_PATH], szClass[MAX_PATH];
		do {

			::GetWindowText(hCWnd, szText, MAX_PATH);
			::GetClassName(hCWnd, szClass, MAX_PATH);
			StringCchPrintf(szTitle, MAX_PATH, TEXT("%#08x|%s|%s"), hCWnd, szClass, szText);

			int nImage = GetTypeIconCode(hCWnd, szTitle);

			if (nImage == -1)nImage = 0;

#ifdef _DEBUG
			for (int i = 0; i < state; i++)
				_tprintf(TEXT("    "));
			_tprintf(TEXT("%s nImage = %d\n"), szTitle, nImage);
#endif
			HTREEITEM NewItem = mTvWindows.InsertItem(szTitle, nImage, nImage, faItem);
			bFind = bFind || EnumWindowChlid(::GetWindow(hCWnd, GW_CHILD), hWnd, NewItem, state + 1);
			if (hCWnd == hWnd)
			{
				bFind = true;
				mTvWindows.SelectItem(NewItem);
				mTvWindows.Expand(NewItem, TVE_EXPAND);
				//::SetFocus(mTvWindows);
			}
			hCWnd = ::GetWindow(hCWnd, GW_HWNDNEXT);
		} while (hCWnd != NULL && faItem != TVI_ROOT);
	}
	return bFind;
}
void CWndPreviewDlg::OnLvnColumnclickListChildview(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	for (int i = 0; i < mLvChildWnd.GetItemCount(); ++i) //这个for用来给表中的项添加上索引号
	{
		mLvChildWnd.SetItemData(i, i);
	}

	m_nfav = !m_nfav;

	DATA data = { m_nfav,pNMLV->iSubItem,&mLvChildWnd };

	mLvChildWnd.SortItems(ListCompare, (LPARAM)&data);

	*pResult = 0;
}
int	CALLBACK CWndPreviewDlg::ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	DATA* pData = (DATA*)lParamSort;

	int col = pData->iSubItem;//点击的列项传递给col，用来判断点击了第几列

	CListCtrl*	pListCtrl = pData->pListCtrl;

	int cmp = lstrcmp(pListCtrl->GetItemText(int(lParam1), col), pListCtrl->GetItemText(int(lParam2), col));

	if (cmp > 0)return 1 - pData->nfav * 2;
	else if (cmp < 0)return pData->nfav * 2 - 1;
	else return int(cmp);

	return 0;
}

void CWndPreviewDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialog::OnMouseMove(nFlags, point);
	// TODO: 在此添加消息处理程序代码和/或调用默认值

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

			if (point.x < 5)point.x = 5;
			else if (point.x > cx - 5)point.x = cx - 5;

			m_TreeProp = point.x;

			this->UpdateWindowsFrame(cx, cy);

			if (m_pIFileEngine != NULL)
			{
				m_pIFileEngine->SetVarValue(IFileEngine::VT_LONG, FV_PREVIEW_LONG_TREEPROP, &m_TreeProp);
			}
		}
		else if (bin == false)
		{
			::ReleaseCapture();
		}
	}
}

void CWndPreviewDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_hWnd == ::GetCapture())
	{
		m_SetSize = true;
	}
}

void CWndPreviewDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonUp(nFlags, point);
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_hWnd == ::GetCapture() || m_SetSize == true)
	{
		m_SetSize = false;
		::ReleaseCapture();
	}
}

void CWndPreviewDlg::OnTvnSelchangedTreeView(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	/*
	GetChildItem 获取一个指定tree view项的子项
	GetNextSiblingItem 获取指定tree view项的下一个兄弟项
	GetPrevSiblingItem 获取指定tree view项的前一个兄弟项
	*/
	if (m_Filter > 0)
	{
		m_Filter--;
		return;
	}

	mLvChildWnd.DeleteAllItems();

	HTREEITEM sht = mTvWindows.GetSelectedItem();

	CString	szItemText;

	int iCount = 0, nImage = 0, nSelectedImage = 0;


	szItemText = mTvWindows.GetItemText(sht);
	mTvWindows.GetItemImage(sht, nImage, nSelectedImage);

#ifdef _WIN64
	AddWndToListCtrl(HWND(_tcstoll(szItemText, NULL, 16)), 0, nImage, szItemText);
#else
	AddWndToListCtrl(HWND(_tcstol(szItemText, NULL, 16)), 0, nImage, szItemText);
#endif // _WIN64

	for (HTREEITEM ht = mTvWindows.GetChildItem(sht); ht != NULL;)
	{
		mTvWindows.GetItemImage(ht, nImage, nSelectedImage);

		szItemText = mTvWindows.GetItemText(ht);

#ifdef _WIN64
		AddWndToListCtrl(HWND(_tcstoll(szItemText, NULL, 16)), iCount, nImage, szItemText);
#else
		AddWndToListCtrl(HWND(_tcstol(szItemText, NULL, 16)), iCount, nImage, szItemText);
#endif // _WIN64

		iCount++;
		ht = mTvWindows.GetNextSiblingItem(ht);
	}
	CString	szStr;
	szStr.Format(TEXT("窗口有%d个子窗口"), iCount);
	mSbStatus.SetWindowText(szStr);

	*pResult = 0;
}

void CWndPreviewDlg::AddWndToListCtrl(HWND hWnd, int nItem, int nImage, CString &szItemText)
{
	static long		n1, n2;
	static DWORD	dwPid;
	static CString	szTemp;
	static WndInfoElem*	pWndInfoElem = NULL;

	mLvChildWnd.InsertItem(nItem, TEXT("窗口不存在"), nImage);

	if (::IsWindow(hWnd))
	{
		szTemp.Format(TEXT("%#08x|%d"), hWnd, hWnd);

		mLvChildWnd.SetItemText(nItem, 0, szTemp);//Handle

		clock_t start = clock();

		n1 = szItemText.Find('|');
		n2 = szItemText.Find('|', n1 + 1);

		mLvChildWnd.SetItemText(nItem, 1, szItemText.Right(szItemText.GetLength() - n2 - 1));//Clas
		mLvChildWnd.SetItemText(nItem, 2, szItemText.Mid(n1 + 1, n2 - n1 - 1));//Text


		::GetWindowThreadProcessId(hWnd, &dwPid);
		szTemp.Format(TEXT("%#08x|%d"), dwPid, dwPid);
		mLvChildWnd.SetItemText(nItem, 3, szTemp);//Pid

		pWndInfoElem = this->GetExeFile(hWnd, szItemText);
		if (pWndInfoElem != NULL)
		{
			mLvChildWnd.SetItemText(nItem, 4, pWndInfoElem->szFile);
			mLvChildWnd.SetItemText(nItem, 6, pWndInfoElem->szPath);
		}
		pWndInfoElem = this->GetModuleFile(hWnd, szItemText);
		if (pWndInfoElem != NULL)
		{
			mLvChildWnd.SetItemText(nItem, 5, pWndInfoElem->szFile);
			mLvChildWnd.SetItemText(nItem, 7, pWndInfoElem->szPath);
		}
	}
	//else
	//{
	//	mLvModule.SetItemText(nItem,0,"窗口不存在");
	//}

}

void CWndPreviewDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: 在此处添加消息处理程序代码
	if (pWnd->m_hWnd == mLvChildWnd.m_hWnd)
	{
		POINT	pt = { point.x,point.y };
		::ScreenToClient(mLvChildWnd.m_hWnd, &pt);

		int index = mLvChildWnd.GetSelectionMark();

		RECT	rc;
		mLvChildWnd.GetItemRect(index, &rc, 0);

		//HDC hdc = ::GetDC(pWnd->m_hWnd);
		//::FillRect(hdc,&rc,GetStockBrush(BLACK_BRUSH));
		//::ReleaseDC(pWnd->m_hWnd,hdc);

		if (PtInRect(&rc, pt))
		{
			CMenu*	pMenu = this->GetMenu();
			pMenu->GetSubMenu(1)->TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
		}

	}
}

void CWndPreviewDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);

	// TODO: 在此处添加消息处理程序代码

	if (m_pIFileEngine != NULL)
	{
		m_pIFileEngine->SetVarValue(IFileEngine::VT_STRUCT, FV_PREVIEW_STRUCT_WRECT, pRect);
	}

}

void CWndPreviewDlg::OnNMDblclkListChildview(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	CString	szItemText;
	TCHAR	szhWnd[MAX_PATH] = TEXT("");
	int index = mLvChildWnd.GetSelectionMark();
	if (index != -1)
	{
		mLvChildWnd.GetItemText(index, 0, szhWnd, 256);
		szItemText = szhWnd;
	}
	else
	{
		HTREEITEM sht = mTvWindows.GetSelectedItem();
		if (sht != NULL)
			szItemText = mTvWindows.GetItemText(sht);
	}

#ifdef _WIN64
	HWND hWnd = HWND(_tcstoll(szItemText, NULL, 16));
#else
	HWND hWnd = HWND(_tcstol(szItemText, NULL, 16));
#endif // _WIN64

	CRect	cRect, dRect;
	::GetWindowRect(hWnd, &cRect);
	::GetWindowRect(::GetDesktopWindow(), &dRect);

	if (::IsIconic(hWnd)) {
		_tprintf(TEXT("窗口已经最小化。\n"));
	}
	//else if (cRect.left > dRect.right || cRect.right < dRect.left || cRect.top > dRect.bottom || cRect.bottom < dRect.top)
	//{
	//	_tprintf(TEXT("窗口不在可视范围内。\n"));
	//}
	else {
		m_pIWndSigned->SignRect(&cRect, 2400);
	}

}
