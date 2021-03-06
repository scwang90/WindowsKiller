#pragma once

#include "resource.h"

interface RFindWindow
{
	virtual void OnFindWindow(HWND hwnd) = 0;
	virtual void OnEndFindWindow() = 0;
	//If the normal calls, should return true
	virtual bool OnCheckRFindWindow() = 0;
};

// CWndWizFindWindowDlg 对话框

class CWndWizFindWindowDlg : public CDialog
{
	DECLARE_DYNAMIC(CWndWizFindWindowDlg)

public:
	CWndWizFindWindowDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWndWizFindWindowDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FINDWINDOW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

public:
	// modal processing
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);

	// 实现
protected:
	HICON	m_hIcon;
	HWND	m_hFwnd;

protected:
	RFindWindow * m_pRFindWindow;
	IWndEngine*		mPtrWndEngine;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

protected:
	DECLARE_MESSAGE_MAP()

public:
	bool SetRFindWindow(RFindWindow* pRFindWindow);
	void SetFindParam(LPCTSTR lpClass, LPCTSTR lpTitle, HWND hwnd);

public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonOkCancel();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedButtonFind();

	afx_msg void OnBnClickedCheckFuzzy();
	afx_msg void OnBnClickedCheckCasematter();
};
