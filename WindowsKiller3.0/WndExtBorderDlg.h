#pragma once

#include "resource.h"
#include "IWndExtract.h"
#include "WndExtLockWizardDlg.h"

// CWndExtBorderDlg 对话框

class CWndExtBorderDlg : public CDialog, public RLockWizardDlg
{
	DECLARE_DYNAMIC(CWndExtBorderDlg)

public:
	CWndExtBorderDlg(IWndExtract*	pIWndExtract, HWND hWnd, CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWndExtBorderDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BORDER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

public:
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);

	virtual void OnLockRect(LPRECT lpRect);
	//If the normal calls, should return true
	virtual bool OnCheckRLockWizardDlg();

	// 实现
protected:
	HICON	m_bIcon;
	HICON	m_sIcon;
	HWND	m_hCwnd;
	HWND	m_hPwnd;
	CRect	m_CRect;
	CRect	m_lRect;
	bool	m_fTopMost;
	bool	m_flock;
	UINT	m_uElapse;

	CWndExtLockWizardDlg m_LockWizardDlg;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

protected:
	DECLARE_MESSAGE_MAP()

protected:
	IWndExtract * m_pIWndExtract;
	ITaskbarList*	m_pITaskbar;

protected:
	void Restore();

public:
	HWND GetChild() { return m_hCwnd; }

public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonOkCancel();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
