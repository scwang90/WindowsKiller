#pragma once

#include "WndAttClassDlg.h"
#include "WndAttGeneralDlg.h"
#include "WndAttProcessDlg.h"
#include "WndAttStylesDlg.h"
#include "WndAttWindowDlg.h"

#include "IWndAttribute.h"

// CWndAttributeDlg 对话框

class CWndAttributeDlg : public CDialog, public IWndAttribute
{
	DECLARE_DYNAMIC(CWndAttributeDlg)

public:
	CWndAttributeDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWndAttributeDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WINATT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

														// 操作
public:
	// IUnknown member function
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void **ppv);
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

	// IWndPreview member function	
	virtual HWND GetWndHandle();
	virtual bool QueryWindowAttrib(HWND hWnd);
	virtual bool CreateAttributeView(CWnd* pParentWnd);
	virtual bool SetIFileEngine(IFileEngine*pIFileEngine);

private:
	// IUnknown member data	
	int		m_Ref;

	// IWndPreview member function

public:
	static	BOOL CALLBACK SetChildFont(HWND hWnd, LPARAM lparam);
	virtual	BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	// Implementation
protected:
	HICON	m_hIcon;
	HWND	m_hCWnd;
	CFont	m_cFont;

	// Generated message map functions
	//{{AFX_MSG(CAttributeDlg)
	virtual BOOL OnInitDialog();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnCheckWindow(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnCancel();
	afx_msg void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	bool	SetCheckWindow(HWND hWnd);

protected:
	IWndEngine * mPtrWndEngine;

private:
	CTabCtrl			m_ctrlTab;
	CPtrArray			m_DlgArray;
	CWndAttClassDlg		m_WndAttClassDlg;
	CWndAttGeneralDlg	m_WndAttGeneralDlg;
	CWndAttProcessDlg	m_WndAttProcessDlg;
	CWndAttStylesDlg	m_WndAttStylesDlg;
	CWndAttWindowDlg	m_WndAttWindowDlg;

	afx_msg void OnBnClickedRefresh();
};
