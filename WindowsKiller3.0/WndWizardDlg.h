#pragma once

#include "IWndWizard.h"
#include "WndWizFindWindowDlg.h"
#include "WndWizFindOptionDlg.h"

// CWndWizardDlg 对话框

class CWndWizardDlg : public CDialog, public RFindWindow, public RWndPreview
{
	DECLARE_DYNAMIC(CWndWizardDlg)

public:
	CWndWizardDlg(IWndWizard* pIViewWizard, bool isBack, CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWndWizardDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WIZARD_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

public:
	virtual BOOL	Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	virtual void	OnFindWindow(HWND hwnd);
	virtual bool	OnCheckRFindWindow();
	virtual void	OnEndFindWindow();
	virtual bool	OnQueryWindow(HWND hwnd);
	virtual bool	OnQueryProcess(DWORD dwPid);
	virtual bool	OnCheckRWndPreview();
	virtual LPCTSTR	OnGetLastError();

public:
	static bool SetIWndModule(IWndModule*pIWndModule);
	static bool SetIWndPreview(IWndPreview*pIWndPreview);
	static bool SetIFileEngine(IFileEngine*pIFileEngine);

	// 实现
protected:
	HICON		m_hIcon;
	CFont		m_cFont;
	ULONG		m_nShow;
	CRect		m_wRect;
	bool		m_bIsTopMost;
	bool		m_bIsEnGary;
	bool		m_bIsHideMode;
	DWORD		m_lSignColor;
	BOOL		m_bIsBackstage;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	static	BOOL CALLBACK SetChildFont(HWND hwnd, LPARAM lparam);


	DECLARE_MESSAGE_MAP()

protected:
	void	UpDateUserInterface();
	void	ReleaseMouseCapture();
	void	BeginFind();
	void	BeginMove();
	void	EndFind();
	void	EndMove();

protected:
	enum Style { STYLE_NULL, STYLE_FIND, STYLE_MOVE };
	int		m_Style;

	WORD	m_wMainPanel1;
	WORD	m_wMainPanel2;
	WORD	m_wBackBegin1;
	WORD	m_wBackBegin2;
	WORD	m_wBackEnd1;
	WORD	m_wBackEnd2;
	WORD	m_wEndFind1;
	WORD	m_wEndFind2;

	WORD	m_wFindCpu1;
	WORD	m_wFindCpu2;

protected:
	CStatic		mCurFind;
	CStatic		mCurMove;
	HCURSOR		mHcurFinder;
	HCURSOR		mHcurEmpty;
	HCURSOR		mHcurHander;
	DWORD		m_dwPreModulePid;
	HWND		m_hPreViewHwnd;

	CToolTipCtrl 	m_ToolTip;
	CHotKeyCtrl		mCtlHotKey;
	CMFCMenuBar		mMfcMenuBar;
	CWndWizFindWindowDlg	m_FindWindowDlg;
	CWndWizFindOptionDlg	m_FindOptionDlg;

protected:
	IWndEngine *	mPtrWndEngine;
	IWndSigned*		m_pIWndSigned;
	IWndExtract*	m_pIWndExtract;
	IWndWizard*		m_pIViewWizard;
	IWndAttribute*	m_pIWndAttrib;


protected:
	static	IWndPreview*	m_pIWndPreview;
	static	IWndModule*		m_pIWndModule;
	static	IFileEngine*	m_pIFileEngine;
	static  IProcEngine*	m_pIProcEngine;

public:
	afx_msg void OnStnClickedFind();
	afx_msg void OnStnClickedMove();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnClose();
	afx_msg void OnBnClickedTopmost();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButtonChangle();
	afx_msg void OnBnClickedButtonUpdate();
	afx_msg void OnBnClickedButtonToparent();
	afx_msg void OnBnClickedButtonOkCancel();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedAutoMode();
	afx_msg void OnBnClickedButtonPickup();
	afx_msg void OnBnClickedButtonFindwindow();
	afx_msg void OnBnClickedGray();
	afx_msg BOOL OnQueryOpen();
	afx_msg	LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedHidemode();
	afx_msg void OnSetHotkey();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnBnClickedButtonSendmesage();
	virtual BOOL DestroyWindow();
};
