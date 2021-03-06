#pragma once

#include "resource.h"

interface RLockWizardDlg
{
	virtual void OnLockRect(LPRECT lpRect) = 0;
	//If the normal calls, should return true
	virtual bool OnCheckRLockWizardDlg() = 0;
};

// CWndExtLockWizardDlg 对话框

class CWndExtLockWizardDlg : public CDialog
{
	DECLARE_DYNAMIC(CWndExtLockWizardDlg)

public:
	CWndExtLockWizardDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWndExtLockWizardDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOCKWIZARD_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

public:
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);

	// 实现
protected:

	CPen	m_cPen;
	CBrush	m_cBrush;
	CBrush	m_cBrishPos;
	CBrush	m_cDlgBack;

	CRect	m_sRect;//计算矩形
	CRect	m_wRect;//
	CRect	m_dRect;
	CRect	m_bRect;//窗口客户矩形

	CDC		m_DeskDC;//桌面源
	CDC		m_hWndDC;//窗口源
	CDC		m_memHDC;//缓冲
	CDC		m_CalHDC;//计算窗口

	CBitmap	m_memHMap;//缓冲
	CBitmap	m_DeskMap;//桌面源
	CBitmap	m_hWndMap;//窗口源
	CBitmap	m_CalcMap;//计算窗口

	UINT	m_CurStyle;
	UINT	m_OppStyle;

	CPoint	m_OffPos;

	CString	m_szSize;

	CMenu	m_cMenu;

	HWND	m_hCWnd;
	HWND	m_hBWnd;

	RLockWizardDlg*	m_pRLockWizardDlg;

protected:
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()

protected:
	void	UpDateWindowPaint();
	bool	AdsorptionVariable(long *Var, long Ads, long offset = 8);
	void	CalculateWindowRect(CPoint &point);
	void	CursorStyleCheck(bool bl, bool bt, bool br, bool bb, CPoint &point);
	void	CursorStyleCancel(bool bl, bool bt, bool br, bool bb, CPoint &point);

public:
	bool	WizardWndRect(HWND hCWnd, HWND hBWnd);
	bool	SetRLockWizardDlg(RLockWizardDlg* pRLockWizardDlg);

public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
