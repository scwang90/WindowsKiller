#pragma once

#include "Resource.h"


// CWndAttStylesDlg 对话框

class CWndAttStylesDlg : public CDialog
{
	DECLARE_DYNAMIC(CWndAttStylesDlg)

public:
	CWndAttStylesDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWndAttStylesDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_STYLES };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

public:
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);


public:
	bool	SetIWndEngine(IWndEngine* pIWndEngine);
	void	UpdateWindowInfo();

protected:
	void	UpdateUI(DWORD ItemId, CListBox &cListBox, CString &cString);

protected:
	CListBox	m_ListStyle;
	CListBox	m_ListExStyle;

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
