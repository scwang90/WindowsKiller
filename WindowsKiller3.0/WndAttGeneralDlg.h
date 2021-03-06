#pragma once

#include "Resource.h"


// CWndAttGeneralDlg 对话框

class CWndAttGeneralDlg : public CDialog
{
	DECLARE_DYNAMIC(CWndAttGeneralDlg)

public:
	CWndAttGeneralDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWndAttGeneralDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_GENERAL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

public:
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);


public:
	bool	SetIWndEngine(IWndEngine* pIWndEngine);
	void	UpdateWindowInfo();
	void	UpdateWindowRect(LPRECT lpRect, DWORD iTemId);

protected:
	IWndEngine * mPtrWndEngine;


	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonOkCancel();

};
