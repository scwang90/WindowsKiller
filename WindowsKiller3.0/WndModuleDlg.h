#pragma once

#include "IWndModule.h"
// CWndModuleDlg 对话框

class CWndModuleDlg : public CDialog, public IWndModule
{
	DECLARE_DYNAMIC(CWndModuleDlg)

public:
	CWndModuleDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWndModuleDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MODULEVIEW };
#endif

	// 操作
public:
	// IUnknown member function
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void **ppv);
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

	// IWndModule member function	
	virtual HWND GetWndHandle();
	virtual bool CreateWndModule(CWnd* pParentWnd = 0);
	virtual	bool UpdateWndModuleInfo(LPCTSTR lpTstr, HWND hFwnd = NULL, DWORD dwPid = 0);
	void ListModulesForProcess(const DWORD &dwProcessId);
	virtual bool SetCallback(IWndModuleCallback* pRWndModule);
	virtual bool SetIFileEngine(IFileEngine*pIFileEngine);
	virtual LPVOID GetProcEngine();

private:
	// IUnknown member data	
	int		m_Ref;

	// IWndModule member function
	IWndEngine*		mPtrWndEngine;
	IProcEngine*	mPtrProcEngine;
	IWndModuleCallback*		mPtrRWndModule;

	static	IFileEngine*	msPtrIFileEngine;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	static int CALLBACK ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK ProcCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

public:
	// modal processing
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);

	// 实现
protected:

	struct	ImageListElement {
		ImageListElement(LPCTSTR lpStr, DWORD nCode) :szType(lpStr), dwCode(nCode) {}
		ImageListElement() :dwCode(0) {}
		CString	szType;
		DWORD	dwCode;
	};

	DWORD		m_nCode;	//图表数量
	BOOL		m_nfav;		//列表排序
	HICON		m_hIcon;
	HWND		m_hFwnd;	//监视窗口

	CListCtrl	mLvModule;
	CStatic		mSbStatus;
	CListCtrl	mLvProcess;

	CSize		m_ClientSize;
	long		m_TreeProp;
	bool		m_SetSize;

	DWORD		m_dwRemoteId;
	int			m_dwProcItem;	//进程列表排序项
	DWORD		m_dwViewFrame;
	DWORD		m_dwViewStyle;

	CImageList					m_ImageList16;
	CImageList					m_ImageList32;
	CArray<CString>				mArInvalidDll;
	CArray<HICON>				m_ImageIcon;
	CArray<ImageListElement>	m_ImageElement;

protected:

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	void WriteClipboardText(CString &strRow);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

protected:
	DWORD	GetTypeIconCode(LPCTSTR lpStr);
	DWORD	GetTypeIconCode(LPCTSTR lpExt, LPCTSTR lpFileName);
	DWORD	AddTypeIconCode(LPCTSTR lpExt, LPCTSTR lpFileName);
	DWORD	AddFileIconCode(LPCTSTR lpFileName);
	//long	AnalyseModule(LPCTSTR lpStr);
	long	AnalyseModule(LPMI lpmi, UINT num);
	bool	RemoteIdFreeLibrary(LPTSTR szDllName);
	void	UpdateWinProcessInfo(HWND hwnd, DWORD dwPid = 0);
	void	AddListItemForProcess(PCPI lpcProcess);
	void	PutListItemForProcess(int index, PCPI lpcProcess);
	void	UpdateWindowsFrame(int cx, int cy);

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonOkCancel();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnLvnColumnClickListModuleView(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNMDblclkListModuleView(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomDrawListModuleView(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLbnDblclkListProcessView();
	afx_msg void OnNMDblclkListProcessView(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnClickListProcessView(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void UpdateProcessList();
};
