#pragma once

#include "Resource.h"
#include "LinkStatic.h"

// CWndAttWindowsDlg 对话框

class CWndAttWindowDlg : public CDialog
{
	DECLARE_DYNAMIC(CWndAttWindowDlg)

public:
	CWndAttWindowDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWndAttWindowDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_WINDOWS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

public:
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);

public:
	bool	SetIWndEngine(IWndEngine* pIWndEngine);
	void	UpdateWindowInfo();

protected:
	IWndEngine * mPtrWndEngine;

protected:
	CLinkStatic	m_HangleNext;
	CLinkStatic	m_HanglePrev;
	CLinkStatic	m_HangleParent;
	CLinkStatic	m_HangleFirst;
	CLinkStatic	m_HangleOwner;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonOkCancel();
};
