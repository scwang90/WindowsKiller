#pragma once

#include "IWndPreview.h"

typedef struct ImageElem {
	ImageElem(LPCTSTR lpStr, DWORD nCode, DWORD _dwPid = 0)
	{
		szType = lpStr;
		dwPid = _dwPid;
		dwCode = nCode;
	}
	ImageElem()
	{
		dwCode = 0;
		dwPid = 0;
	}

	CString	szType;
	DWORD	dwPid;
	DWORD	dwCode;
}ImageElem;
typedef struct WndInfoElem {
	WndInfoElem(LPCTSTR lpStr, DWORD _dwPid)
	{
		szPath = lpStr;
		szFile = GetFileName(lpStr);
		dwPid = _dwPid;
	}
	WndInfoElem(LPCTSTR lpStr, HMODULE _hModule)
	{
		szPath = lpStr;
		szFile = GetFileName(lpStr);
		hModule = _hModule;
	}
	WndInfoElem(LPCTSTR lpStr, HMODULE _hModule, DWORD _dwPid)
	{
		szPath = lpStr;
		szFile = GetFileName(lpStr);
		hModule = _hModule;
		dwPid = _dwPid;
	}
	WndInfoElem()
	{
		hModule = NULL;
	}
	int StrFind(LPCTSTR lpStr, TCHAR ch, int start)
	{
		for (int n = start; lpStr[n] != 0; n++)
		{
			if (lpStr[n] == ch)
				return n;
		}
		return -1;
	}
	LPCTSTR GetFileName(LPCTSTR lpPath)
	{
		int n = 0;
		for (int i = 0; (i = StrFind(lpPath, '\\', n)) != -1;)
		{
			n = i + 1;
		}
		return &lpPath[n];
	}

	CString	szFile;
	CString	szPath;

	DWORD	dwPid;
	HMODULE	hModule;
}WndInfoElem;

// CWndPreviewDlg 对话框

class CWndPreviewDlg : public CDialog, public IWndPreview
{
	DECLARE_DYNAMIC(CWndPreviewDlg)

public:
	CWndPreviewDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWndPreviewDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CHILDVIEW };
#endif

	// 操作
public:
	// IUnknown member function
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void **ppv);
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

	// IWndPreview member function	
	virtual HWND GetWndHandle();
	virtual bool ShowChildWndView();
	virtual bool HideChildWndView();
	virtual bool UpdateChildWndsInfo(HWND hWnd);
	virtual bool CreateChildWndView(CWnd* pParentWnd);
	virtual bool SetRWndPreview(RWndPreview* pRWndPreview);
	virtual bool DelRWndPreview(RWndPreview* pRWndPreview);
	virtual bool SetIFileEngine(IFileEngine*pIFileEngine);

private:
	// IUnknown member data	
	int		m_Ref;

	// IWndPreview member function
	HWND			m_hFwnd;
	long			m_Filter;

	IWndEngine*		mPtrWndEngine;
	IWndSigned*		m_pIWndSigned;
	RWndPreview*	m_pRWndPreview;

	static	IFileEngine*	m_pIFileEngine;

	static	long				m_nCode;
	static	CImageList			m_ImageList;
	static	CArray<ImageElem>	m_ElemImage;
	static	CArray<WndInfoElem>	m_ElemModule;
	static	CArray<WndInfoElem>	m_ElemPeFile;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	// modal processing
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	static int CALLBACK ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	// 实现
protected:
	HICON		m_hIcon;
	BOOL		m_nfav;
	CStatic		mSbStatus;
	CListCtrl	mLvChildWnd;
	CTreeCtrl	mTvWindows;
	CSize		m_ClientSize;
	long		m_TreeProp;
	bool		m_SetSize;


	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	DECLARE_MESSAGE_MAP()

protected:
	WndInfoElem * AddExeFile(HWND hWnd, DWORD dwPid);
	WndInfoElem*	GetExeFile(DWORD dwPid);
	WndInfoElem*	GetExeFile(HWND hWnd, LPCTSTR lphWnd);
	WndInfoElem*	AddtModuleFile(HWND hWnd, HMODULE hModule, DWORD dwPid);
	WndInfoElem*	GetModuleFile(HMODULE hModule, DWORD dwPid);
	WndInfoElem*	GetModuleFile(HWND hWnd, LPCTSTR lphWnd);

	HTREEITEM	FindItemFormTree(HTREEITEM hTreeItem, HWND hWnd);

	long	AddItemSameAsProc(HTREEITEM hTreeItem, DWORD dwPid);

	DWORD	GetTypeIconCode(HWND hWnd, LPCTSTR lpStr, DWORD dwPid);
	DWORD	GetTypeIconCode(HWND hWnd, LPCTSTR lphWnd);
	DWORD	AddTypeIconCode(HWND hWnd, LPCTSTR lpFileName, DWORD dwPid = 0);
	DWORD	AddHandIconCode(HWND hWnd, LPCTSTR lphWnd);
	DWORD	AddFileIconCode(LPCTSTR lpFileName, DWORD dwPid = 0);
	void	AddWndToListCtrl(HWND hWnd, int nItem, int nImage, CString &szItemText);
	bool	EnumWindowChlid(HWND hPnd, HWND hWnd, HTREEITEM faItem = TVI_ROOT, int state = 0);
	void	UpdateWindowsFrame(int cx, int cy);

public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonOkCancel();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnLvnColumnclickListChildview(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTvnSelchangedTreeView(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnNMDblclkListChildview(NMHDR *pNMHDR, LRESULT *pResult);
};
